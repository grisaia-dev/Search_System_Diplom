#include <iostream>
#include <SSNetworking/server.hpp>
#include <SSNetworking/session.hpp>
#include <SSHelp/hit.hpp>

namespace SS {
    using boost::asio::ip::tcp;
    Server::Server(IPV ipv, unsigned short port) : _ipVersion(ipv), _port(port),
        _acceptor(_ioContext, tcp::endpoint(_ipVersion == IPV::V4 ? tcp::v4() : tcp::v6(), _port)) {}

    int Server::run() {
        try {
            startAccept();
            _ioContext.run();
        } catch (const std::exception& ex) {
            std::cerr << M_ERROR << ex.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    void Server::startAccept() {
        auto connection = Session::create(_ioContext);

        _connections.push_back(connection);
        _acceptor.async_accept(connection->get_socket(),
            [connection, this](const boost::system::error_code& error) {
                if (!error) {
                    connection->start();
                }
                startAccept();
            });
    }
}