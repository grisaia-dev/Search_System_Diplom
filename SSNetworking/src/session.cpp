#include <SSNetworking/session.hpp>
#include <SSHelp/hit.hpp>
#include <SSDataBase/db.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <string>
#include <iosfwd>
#include <ios>
#include <vector>

namespace SS {
    std::string string_to_utf_8(const std::string str) {
        std::string res;
	    std::istringstream iss(str);
        char ch;
        while (iss.get(ch)) {
            if (ch == '%') {
                int hex;
                iss >> std::hex >> hex;
                res +=static_cast<char>(hex);
            } else {
                res += ch;
            }
        }
        return res;
    }

    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    using tcp = boost::asio::ip::tcp;
    Session::Session(boost::asio::io_context& ioContext) : _socket(ioContext) {}

    void Session::start(const Config& conf) {
        _config = conf;
        read_request();
    }

    void Session::read_request() {
        auto self = shared_from_this();

        http::async_read(_socket, _buffer, _request, 
            [self](const beast::error_code& error, size_t bytesTransferred) {
                if (!error) {
                    std::cout << M_ENTER << "Accepted " << bytesTransferred << " bytes of data" << std::endl;
                    std::cout << self->_request.base();
                    self->parse_request();
                } else {
                    std::cerr << M_ERROR << error.what() << std::endl;
                }
            });
    }

    void Session::parse_request() {
        _response.version(_request.version());
	    _response.keep_alive(false);
        switch (_request.method()) {
            case http::verb::get:
                _response.result(http::status::ok);
                _response.set(http::field::server, "Beast");
                response_get();
                break;
            case http::verb::post:
                _response.result(http::status::ok);
                _response.set(http::field::server, "Beast");
                response_post();
                break;
            default:
                _response.result(http::status::bad_request);
                _response.set(http::field::content_type, "text/plain");
                beast::ostream(_response.body())
			        << "Invalid request-method '"
			        << std::string(_request.method_string()) << "'";
                break;
        
        }
        response_write();
    }

    void Session::response_get() {
        if (_request.target() == "/") {
            _response.set(http::field::content_type, "text/html");
            beast::ostream(_response.body()) 
                << "<html>\n"
			    << "<head><meta charset=\"UTF-8\"><title>Search Engine</title></head>\n"
			    << "<body>\n"
			    << "<h1>Search Engine</h1>\n"
			    << "<form action=\"/\" method=\"post\">\n"
			    << "\t<label for=\"search\">Search:</label><br>\n"
			    << "\t<input type=\"text\" id=\"search\" name=\"search\"><br>\n"
			    << "\t<input type=\"submit\" value=\"Search\">\n"
			    << "</form>\n"
			    << "</body>\n"
			    << "</html>\n";
        } else {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "File not found\r\n";
        }
    }

    void Session::response_post() {
        db database;
        try {
            database.set_connect_string(_config.dbHost, _config.dbPort, _config.dbName, _config.dbUser, _config.dbPass);
            database.connect();
        } catch (const std::exception& ex) { std::cerr << M_ERROR << ex.what() << std::endl; }

        if (database.is_open()) {
            if (_request.target() == "/") {
                std::string req_data = buffers_to_string(_request.body().data());
                std::cout << M_ENTER << "Post query: " << req_data << std::endl;

                size_t pos = req_data.find('=');
                if (pos == std::string::npos) {
                    _response.result(http::status::not_found);
                    _response.set(http::field::content_type, "text/plain");
                    beast::ostream(_response.body()) << "Unknow query!" << std::endl;
                    return;
                }

                std::string key = req_data.substr(0, pos);
                std::string value = req_data.substr(pos + 1);
                value = string_to_utf_8(value);
                boost::algorithm::to_lower(value);

                if (key != "search") {
                    _response.result(http::status::not_found);
			        _response.set(http::field::content_type, "text/plain");
			        beast::ostream(_response.body()) << "Unknow query!\r" << std::endl;;
			        return;
                }

                std::regex word_reg("[\\w{2,30}]+");
                std::sregex_iterator words_begin = std::sregex_iterator(value.begin(), value.end(), word_reg);
                std::sregex_iterator words_end = std::sregex_iterator();
                int n_word = std::distance(words_begin, words_end);

                std::vector<std::string> search_result;
                if (n_word > 0) {
                    std::vector<std::string> words;
                    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                        std::smatch match = *i;
                        if (match.str().size() > 2) words.push_back(match.str());
                    }

                    std::vector<int> word_id;
                    for (int i = 0; i < words.size(); ++i) { word_id.push_back(database.get_id_word(words[i])); }

                    std::map<int, int> reng;
                    std::map<int, int>::iterator it;
                    if (word_id.size() != 0) {
                       for (int i = 0; i < n_word; ++i) {
                           if (word_id[i] != 0) {
					            auto documents = database.get_word_count(word_id[i]);
					            for (auto doc : documents) {
						         it = reng.find(doc.first);
						         if (it != reng.end()) { reng[doc.first] += doc.second; }
						         else { reng[doc.first] = doc.second; }
					            }
				            }     
                       }
                    }
                    if (reng.size() != 0) {
				        std::multimap<int, int> revers_map;
				        for (const auto& element : reng) {
				        	revers_map.insert({ element.second, element.first });
				        }

				        // TODO: Fetch your own search results here
				        int i = 0;
				        for (auto iter = revers_map.end(); iter != revers_map.begin();) {
				        	iter--;
				        	if (i < 10) {
				        		Link l = database.get_link(iter->second);
				        		std::string str = "";
				        		str = l.protocol + l.host + l.query;
				        		search_result.push_back(str);
				        		i++;
				        	}
				        }
			        }
                }
                _response.set(http::field::content_type, "text/html");
                beast::ostream(_response.body())
                    << "<html>\n"
			        << "<head><meta charset=\"UTF-8\"><title>Search Engine</title></head>\n"
			        << "<body>\n"
			        << "<h1>Search Engine</h1>\n"
			        << "<p>Response:<p>\n"
			        << "<ul>\n";

                if (search_result.size() != 0) {
                    for (const auto& url : search_result) {
                        beast::ostream(_response.body())
                            << "<li><a href=\""
					        << url << "\">"
					        << url << "</a></li>";
                    }
                } else {
                    beast::ostream(_response.body()) << "<p>No words were found for your query</p>";
                }
                beast::ostream(_response.body()) << "</ul>\n" << "</body>\n" << "</html>\n";
            } else {
                _response.result(http::status::not_found);
		        _response.set(http::field::content_type, "text/plain");
		        beast::ostream(_response.body()) << "File not found\r" << std::endl;
            }
        } else {
            _response.result(http::status::failed_dependency);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "Can't connect to data base!\r" << std::endl;
        }
    }

    void Session::response_write() {
        auto self = shared_from_this();

        _response.content_length(_response.body().size());
        http::async_write(_socket, _response, 
            [self](const beast::error_code& error, size_t bytesTransferred){
                if (error) {
                    std::cerr << M_ERROR << error.what() << std::endl;
                } else {
                    std::cout << M_ENTER << "Send " << bytesTransferred << " bytes of data" << std::endl;
                    std::cout << self->_response.base();
                    self->_socket.shutdown(tcp::socket::shutdown_send);
                }
            });
    }
}
