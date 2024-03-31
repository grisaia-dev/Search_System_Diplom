#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace SS {
    class __declspec(dllexport) Session;

    enum class __declspec(dllexport) IPV {
        V4,
        V6
    };

    class __declspec(dllexport) Server {
    public:
        Server(IPV ipv, unsigned short port);

        int run();
    private:
        void startAccept();
    private:
        IPV _ipVersion;
        unsigned short _port;

        boost::asio::io_context _ioContext;
        boost::asio::ip::tcp::acceptor _acceptor;

        std::vector<std::shared_ptr<Session>> _connections;
    };
}