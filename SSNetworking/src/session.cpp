#include <SSNetworking/session.hpp>
#include <SSHelp/hit.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/ostream.hpp>
#include <iostream>


namespace SS {
    namespace beast = boost::beast;
    namespace http = boost::beast::http;
    using tcp = boost::asio::ip::tcp;
    Session::Session(boost::asio::io_context& ioContext) : _socket(ioContext) {}

    void Session::start() {
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
            beast::ostream(_response.body()) << "<html>\n"
			    << "<head><meta charset=\"UTF-8\"><title>Search Engine</title></head>\n"
			    << "<body>\n"
			    << "<h1>Search Engine</h1>\n"
			    << "<p>Welcome!<p>\n"
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
        // TODO
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
