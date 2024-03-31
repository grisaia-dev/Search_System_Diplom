#include <iostream>
#include <SSNetworking/server.hpp>
#include <SSParserConfig/config_parser.hpp>

using boost::asio::ip::tcp;

int main(void) {
    setlocale(LC_ALL, "ru");
    INIP::Parser parser("config.ini");
    SS::Server server(SS::IPV::V4, 1337);
    server.run();

    return EXIT_SUCCESS;
}

