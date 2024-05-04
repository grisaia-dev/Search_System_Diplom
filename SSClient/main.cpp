#include "boost/beast/core/tcp_stream.hpp"
#include "boost/beast/ssl/ssl_stream.hpp"
#include <boost/algorithm/string.hpp>
#include "include/spider.hpp"
//#include <boost/asio/io_context.hpp>
#include <SSHelp/hit.hpp>
#include <SSParserConfig/config_parser.hpp>
#include <SSDataBase/db.hpp>
//#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <openssl/ssl.h>
#include <exception>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <regex>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace http = boost::beast::http;
using boost::asio::ip::tcp;

std::mutex mtx;
std::queue<std::function<void()>> tasks;
std::condition_variable cv;
SS::db database;
bool exit_thread_pool = false;

void thread_pool_worker();
std::string get_html_content(const SS::Link& link);
bool is_text(const boost::beast::multi_buffer::const_buffers_type& b);
std::string remove_html_tags(const std::string s);
std::map<std::string, int> get_words_and_index(const std::string text);
std::vector<std::string> get_html_link(const std::string html);
void parse_link(SS::Link link, int depth);

int main() {
	setlocale(LC_ALL, "ru");
    SS::Config conf;
    INIP::Parser parser("config.ini");
    conf.dbHost = parser.get_value<std::string>("DB.host");
    conf.dbPort = parser.get_value<std::string>("DB.port");
    conf.dbName = parser.get_value<std::string>("DB.name");
    conf.dbUser = parser.get_value<std::string>("DB.user");
    conf.dbPass = parser.get_value<std::string>("DB.password");
    conf.l_protocol = parser.get_value<std::string>("SITE.protocol");
    conf.l_host = parser.get_value<std::string>("SITE.host");
    conf.l_start_page = parser.get_value<std::string>("SITE.page");
    conf.depth = parser.get_value<int>("SITE.depth");

    database.set_connect_string(conf.dbHost, conf.dbPort, conf.dbName, conf.dbUser, conf.dbPass);
    database.connect();
    database.create_structure();
    std::cout << M_HIT << "Starting parse link.." << std::endl;
    try {
        const unsigned short num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> thread_pool;

        for (int i = 0; i < num_threads; ++i) { thread_pool.emplace_back(thread_pool_worker); }

        SS::Link link{ conf.l_protocol, conf.l_host, conf.l_start_page };

        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push([link, conf](){ parse_link(link, conf.depth - 1);});
        }

        //std::this_thread::sleep_for(std::chrono::seconds(2));

		{
			std::lock_guard<std::mutex> lock(mtx);
			exit_thread_pool = true;
			cv.notify_all();
		}

        for (auto& t : thread_pool) {
			t.join();
		}
    
    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << std::endl;
    }
    std::cout << M_GOOD << "Parsing completed!" << std::endl;
    return EXIT_SUCCESS;
}


void thread_pool_worker() {
    std::unique_lock<std::mutex> lock(mtx);
    while (!exit_thread_pool || !tasks.empty()) {
        if (tasks.empty()) {
            cv.wait(lock);
        } else {
            auto task = tasks.front();
            tasks.pop();
            lock.unlock();
            task();
            lock.lock();
        }
    }
}

bool is_text(const boost::beast::multi_buffer::const_buffers_type& b) {
	for (auto itr = b.begin(); itr != b.end(); itr++) {
		for (int i = 0; i < (*itr).size(); i++) {
			if (*((const char*)(*itr).data() + i) == 0)
				return false;
		}
	}
	return true;
}

std::string get_html_content(const SS::Link &link) {
    std::string result;
    try {
        std::string host = link.host;
        std::string query = link.query;

        asio::io_context ioContext;
        if (link.protocol == "https://") {
            ssl::context ctx(ssl::context::tlsv13_client);
            ctx.set_default_verify_paths();

            beast::ssl_stream<beast::tcp_stream> stream(ioContext,ctx);
            stream.set_verify_mode(ssl::verify_none);

            stream.set_verify_callback([](bool preverified, ssl::verify_context& ctx) {
				    return true; // Accept any certificate
				});

            if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
				beast::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
				throw beast::system_error{ec};
			}

            tcp::resolver resolver(ioContext);
			get_lowest_layer(stream).connect(resolver.resolve({ host, "https" }));
			get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

            http::request<http::empty_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            stream.handshake(ssl::stream_base::client);
			http::write(stream, req);

            beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

            if (is_text(res.body().data())) { result = buffers_to_string(res.body().data()); } 
            else { std::cout << M_ERROR << "This is not a text link, bailing out..." << std::endl; }

            beast::error_code ec;
			stream.shutdown(ec);
            if (ec == asio::error::eof) { ec = {}; }

            if (ec) { throw beast::system_error{ec}; }
        } else {
            tcp::resolver resolver(ioContext);
			beast::tcp_stream stream(ioContext);

            auto const results = resolver.resolve(host, "http");
			stream.connect(results);

            http::request<http::string_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            http::write(stream, req);

			beast::flat_buffer buffer;

			http::response<http::dynamic_body> res;

            http::read(stream, buffer, res);

            if (is_text(res.body().data())) { result = buffers_to_string(res.body().data()); }
			else { std::cout << "This is not a text link, bailing out..." << std::endl; }

			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);

			if (ec && ec != beast::errc::not_connected)
				throw beast::system_error{ec};
        }
    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << std::endl;
    }
    return result;
}

std::string remove_html_tags(const std::string s) {
	std::string  str = s;
	int index = s.find("<body");
	str.replace(0,index,"");

	std::regex script("<script[\\s\\S]*?>[\\s\\S]*?<\\/script>", std::regex_constants::ECMAScript);
	str = regex_replace(str, script, " ");

	std::regex pattern_style("<style[^>]*?>[\\s\\S]*?<\\/style>", std::regex_constants::ECMAScript);
	str = regex_replace(str, pattern_style, " ");

	std::regex pattern_a("<a[^>]*?>[\\s\\S]*?<\\/a>", std::regex_constants::ECMAScript);
	str = regex_replace(str, pattern_a, " ");

	std::regex pattern("\\<(\\/?[^\\>]+)\\>", std::regex_constants::ECMAScript);
	str = regex_replace(str, pattern, " ");

	std::regex re("(\\n|\\t|[0-9]|\\s+)", std::regex_constants::ECMAScript);
	str = regex_replace(str, re, " ");

	boost::algorithm::to_lower(str);

	return str;
}

std::map<std::string, int> get_words_and_index(const std::string text) {
	std::regex word_regex("(\\b\\w{3,32}\\b)", std::regex_constants::ECMAScript);
	auto words_begin = std::sregex_iterator(text.begin(), text.end(), word_regex);
	auto words_end = std::sregex_iterator();
	int n = std::distance(words_begin, words_end);
	std::vector<std::string> words;

	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		words.push_back(match.str());
	}

	std::map<std::string, int> indexWords;

	for (int i = 0; i < words.size(); ++i) {
		if (indexWords.find(words[i]) != indexWords.end()) { indexWords[words[i]] += 1; }
		else { indexWords.insert({ words[i], 1}); }
	}
	return indexWords;
}

std::vector<std::string> get_html_link(const std::string html) {
	std::vector<std::string> links;
	std::regex link_regex("<a href=\"(.*?)\"");
	auto link_begin = std::sregex_iterator(html.begin(), html.end(), link_regex);
	auto link_end = std::sregex_iterator();

	std::vector<std::string> words;

	for (std::sregex_iterator i = link_begin; i != link_end; ++i) {
		std::string sTemp;
		std::smatch match = *i;
			if (match[1].matched) sTemp = std::string(match[1].first, match[1].second);
		links.push_back(sTemp);
	}	
	return links;
}

void parse_link(SS::Link link, int depth) {
    try {
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::string html = get_html_content(link);

        if (html.size() == 0) {
			std::cout << M_ERROR << "Failed to get HTML Content" << std::endl;
			return;
		}

        std::string text = remove_html_tags(html);
        std::map<std::string, int> words = get_words_and_index(text);

        if (database.search_link(link)) {
			database.insert_data(words, link);
			std::cout << M_ENTER << "Link parsed: " << link.protocol << link.host << link.query << std::endl;
        } else { std::cout << M_HIT << "Link already in database: " << link.protocol << link.host << link.query << std::endl; }

        std::vector<SS::Link> links;

        std::vector<std::string> linkStr = get_html_link(html);
        for (int i = 0; i < linkStr.size(); ++i ) {
			SS::Link tmp;
			// (http[s]?:\/\/)?(\\w{3}\\.)?(\\w+[\\.\\w+]+)?([/\\\S+]+:?\\w+[\\.\\w+]?(php)?)*(\\?\\S+)?
			std::regex ex("(http[s]?:)*(\\/\\/)?(\\w{3}\\.)?(\\w+[\\.\\w+]+)?([\\/\\w+.?:?=?&?;]*)*(#\\w+)?", std::regex_constants::ECMAScript);
			std::cmatch what;
			if (std::regex_search(linkStr[i].c_str(), what, ex)) {
				if (what[1].matched) {
					tmp.protocol = std::string(what[1].first, what[1].second);
					if (what[2].matched) { tmp.protocol += std::string(what[2].first, what[2].second); }
				} else { tmp.protocol = link.protocol; }		
				if (what[3].matched) { tmp.host = std::string(what[3].first, what[3].second); }
				if (what[4].matched) { tmp.host += std::string(what[4].first, what[4].second); }
				else { tmp.host = link.host; }
				if (what[5].matched) { tmp.query = std::string(what[5].first, what[5].second); }
				links.push_back(tmp);
			}	
		}
        if (depth > 0) {
			std::lock_guard<std::mutex> lock(mtx);

			size_t count = links.size();
			size_t index = 0;
			for (auto& subLink : links) {
				if (database.search_link(subLink))
					tasks.push([subLink, depth]() { parse_link(subLink, depth - 1); });
			}
			cv.notify_one();
		}
    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << std::endl;
    }
}

