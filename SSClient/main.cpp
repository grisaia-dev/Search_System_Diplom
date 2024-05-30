#include <SSParserConfig/config_parser.hpp>
#include <SSDataBase/db.hpp>
#include "include/spider.hpp"
#include <SSHelp/hit.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <openssl/ssl.h>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <condition_variable>
#include <exception>
#include <functional>
#include <codecvt>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <queue>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace http = boost::beast::http;
using boost::asio::ip::tcp;

// Creating pool connections
//SS::db database;
std::mutex mtx;
std::condition_variable cv;
std::queue<std::function<void()>> tasks;
bool exit_thread_pool = false;

std::string get_html_body(const SS::Link& link) {
    std::string result;
    try {
        asio::io_context io_context;
        if (link.protocol == "https://") {
            // create ssl certificate
            ssl::context ctx(ssl::context::tlsv13_client);
            ctx.set_default_verify_paths();
            beast::ssl_stream<beast::tcp_stream> stream(io_context, ctx);
            stream.set_verify_mode(ssl::verify_none);
            stream.set_verify_callback([](bool preverified, ssl::verify_context& ctx) {
                return true; // Accept any certificate
            });
            // if can't get ssl
            if (!SSL_set_tlsext_host_name(stream.native_handle(), link.host.c_str())) {
                beast::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
                throw beast::system_error{ec};
            }

            // create resolver
            tcp::resolver resolver(io_context);
			get_lowest_layer(stream).connect(resolver.resolve({ link.host, "https" }));
			get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

            // Creating request for site
            http::request<http::string_body> req;
            req.method(http::verb::get);
            req.target(link.query);
            req.set(http::field::host, link.host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.prepare_payload();

            // creating handshake and past in request
            stream.handshake(ssl::stream_base::client);
            http::write(stream, req);

            // create buff and response for site / go request for host / reading response
            beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(stream, buffer, res);
            result = beast::buffers_to_string(res.body().data());

            // close ssl stream
            beast::error_code ec;
            stream.shutdown(ec);
            if (ec == asio::error::eof) { ec = {}; }
            if (ec) { throw beast::system_error{ec}; }
        }
        io_context.stop();
    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << "\n";
    }
    return result;
}

void remove_tags(std::string& body) {
    int index = body.find("<body");
    body.replace(0, index, "");

    // firts deleting scripts
    boost::regex script("<script[\\s\\S]*?>[\\s\\S]*?<\/script>", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, script, "");

	// delete style aka CSS
	boost::regex pattern_style("<style[^>]*?>[\\s\\S]*?<\/style>", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, pattern_style, " ");

	// delete links
	boost::regex pattern_a("<a[^>]*?>[\\s\\S]*?<\/a>", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, pattern_a, " ");

	std::regex pattern("\\<(\/?[^\\>]+)\\>", std::regex_constants::ECMAScript);
	body = std::regex_replace(body, pattern, " ");

	std::regex re("(\\n|\\t|[0-9]|\\s+)", std::regex_constants::ECMAScript);
	body = std::regex_replace(body, re, " ");

	// deleting (.-*?) and others
	boost::regex branch("[[:punct:]]", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, branch, "");

	// delete unordynary bracket
	boost::regex lb("«", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, lb, "");

	// aslo too
	boost::regex rb("»", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, rb, "");

	// delete non standarad -
	boost::regex noe("–", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, noe, "");

	// also too
    boost::regex noeb("—", boost::regex_constants::ECMAScript);
	body = boost::regex_replace(body, noeb, "");

	boost::algorithm::to_lower(body);
}

std::map<std::string, int> get_words_and_index(const std::string& body) {
    std::regex word_regex("\\b\\w{3,32}\\b", std::regex_constants::ECMAScript);
    auto words_begin = std::sregex_iterator(body.begin(), body.end(), word_regex);
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

std::vector<SS::Link> get_links(const std::string& body) {
    std::vector<SS::Link> links;
    SS::Link temp;
    std::regex link_regex("<a href=\"(.*?)\"");

    auto link_begin = std::sregex_iterator(body.begin(), body.end(), link_regex);
	auto link_end = std::sregex_iterator();

	for (std::sregex_iterator i = link_begin; i != link_end; ++i) {
		std::string sTemp;
		std::smatch match = *i;
			if (match[1].matched) {
			    sTemp = std::string(match[1].first, match[1].second);
				std::regex ex("https?://([^/]*)?(/.*)", std::regex_constants::ECMAScript);
				std::cmatch what;
				if (std::regex_search(sTemp.c_str(), what, ex)) {
					temp.protocol = "https://";
					if (what[1].matched) { temp.host = std::string(what[1].first, what[1].second); }
					if (what[2].matched) { temp.query = std::string(what[2].first, what[2].second); }
					links.push_back(temp);
				}
			}
	}
	return std::move(links);
}

void parse_link(const SS::Link& link, int depth, const SS::Config& conf) {
    try {
        SS::db database;
        database.set_connect_string(conf.dbHost, conf.dbPort, conf.dbName, conf.dbUser, conf.dbPass);
        database.connect();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // Getting html body
        std::string body = get_html_body(link);
        std::vector<SS::Link> links;

        // if body is empty throw error
        if (body.empty()) { throw std::invalid_argument("[HTML]:Can't connect to link (maybe link is wrong): " + link.protocol+link.host+link.query); }

        // check depth for parsing
        if (depth > 0) {
            links = get_links(body);
            std::lock_guard<std::mutex> lock(mtx);
            size_t count = links.size();
            size_t index = 0;
            for (auto& subLink : links) {
				if (database.search_link(subLink)) {
					tasks.push([subLink, depth, conf]() { parse_link(subLink, depth - 1, conf); });
				}
			}
			cv.notify_one();
        }

        remove_tags(body);
        std::map<std::string, int> words = get_words_and_index(body);

        if (database.search_link(link)) {
            database.insert_data(words, link);
            std::cout << M_ENTER << "Link parsed: " << link.protocol << link.host << link.query << "\n";
        } else { std::cout << M_HIT << "Linnk already in datadase: " << link.protocol << link.host << link.query << "\n"; }

    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << "\n";
    }
}

void thread_pool_worker() {
    std::unique_lock<std::mutex> lock(mtx);
    while (!exit_thread_pool || !tasks.empty()) {
        if (tasks.empty()) { cv.wait(lock); }
        else {
            auto task = tasks.front();
            tasks.pop();
            lock.unlock();
            task();
            lock.lock();
        }
    }
}

int main() {
    try {
        SS::db database;
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

        SS::Link link{conf.l_protocol, conf.l_host, conf.l_start_page};

        // Connecting to databas
        database.set_connect_string(conf.dbHost, conf.dbPort, conf.dbName, conf.dbUser, conf.dbPass);
        database.connect();
        database.delete_structure();
        database.create_structure();

        std::cout << M_HIT << "Starting parse link..\n";

        int num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> thread_pool;

        for (int i = 0; i < num_threads; ++i) { thread_pool.emplace_back(thread_pool_worker); }

        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push([link, conf]() { parse_link(link, conf.depth - 1, conf); });
            cv.notify_one();
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            std::lock_guard<std::mutex> lock(mtx);
            exit_thread_pool = true;
            cv.notify_all();
        }

        for (auto& t : thread_pool) { t.join(); }

        std::cout << M_GOOD << "Parsing completed!\n";
    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << "\n";
    }
    return EXIT_SUCCESS;
}
