#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#ifdef _WIN32
    #define SH_LIB __declspec(dllexport)
#else
    #define SH_LIB
#endif

namespace SS {
    class SH_LIB Session;

    enum class SH_LIB IPV {
        V4,
        V6
    };

    class SH_LIB Server {
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