#include <iostream>
#include <SSHelp/hit.hpp>
#include <SSParserConfig/config_parser.hpp>
#include <SSNetworking/server.hpp>

int main(void) {
    setlocale(LC_ALL, "ru");
    std::cout << M_HIT << "Starting server.." << std::endl;

    INIP::Parser parser("config.ini");
    SS::Config conf;
    conf.dbHost = parser.get_value<std::string>("DB.host");
    conf.dbPort = parser.get_value<std::string>("DB.port");
    conf.dbName = parser.get_value<std::string>("DB.name");
    conf.dbUser= parser.get_value<std::string>("DB.user");
    conf.dbPass= parser.get_value<std::string>("DB.password");
    conf.port = parser.get_value<unsigned short>("SERVER.port");
    parser.~Parser();

    SS::Server server(SS::IPV::V4, conf);
    server.run();

    return EXIT_SUCCESS;
}

