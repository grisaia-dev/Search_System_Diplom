#include "boost/beast/http/write.hpp"
#include <SSNetworking/session.hpp>
#include <SSHelp/hit.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <ostream>


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
                    std::cout << M_ENTER << "Принято " << bytesTransferred << " байт данных" << std::endl;
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
                //response_get();
                break;
            case http::verb::post:
                _response.result(http::status::ok);
                _response.set(http::field::server, "Beast");
                //response_post();
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

    void Session::response_write() {
        auto self = shared_from_this();

        _response.content_length(_response.body().size());
        http::async_write(_socket, _response, 
            [self](const beast::error_code& error, size_t bytesTransferred){
                if (error) {
                    std::cerr << M_ERROR << error.what() << std::endl;
                } else {
                    std::cout << M_ENTER << "Отправленно " << bytesTransferred << " байт данных" << std::endl;
                    self->_socket.shutdown(tcp::socket::shutdown_send);
                }
            });
    }
}
