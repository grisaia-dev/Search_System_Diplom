#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <memory>
#include <string>

#ifdef _WIN32
    #define SH_LIB __declspec(dllexport)
#else
    #define SH_LIB
#endif

namespace SS {
    class SH_LIB Session : public std::enable_shared_from_this<Session> {
    public:
        void start();

        boost::asio::ip::tcp::socket& get_socket() { return _socket; }
        static std::shared_ptr<Session> create(boost::asio::io_context& ioContext) {
            return std::shared_ptr<Session>(new Session(ioContext));
        }
    private:
        explicit Session(boost::asio::io_context& ioContext); // приватно создаем сессию
    private: // Функции
        void read_request(); // Читаем запрос
        void parse_request(); // Парсим запрос на get или post
        void response_get(); // ответ на запрос get
        void response_post(); // ответ на запрос post
        void response_write(); // Отправляем ответ
    private: // Переменные
        boost::asio::ip::tcp::socket _socket;
        boost::beast::flat_buffer _buffer{8192};
        boost::beast::http::request<boost::beast::http::dynamic_body> _request;
        boost::beast::http::response<boost::beast::http::dynamic_body> _response;
    };
}