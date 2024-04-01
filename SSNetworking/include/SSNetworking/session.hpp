#pragma once
#include <memory>
#include <boost/asio/ip/tcp.hpp>
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
        explicit Session(boost::asio::io_context& ioContext);
    private:
        boost::asio::ip::tcp::socket _socket;
        std::string _message;
    };
}