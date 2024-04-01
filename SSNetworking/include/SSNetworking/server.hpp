#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

// Нужно для того, что бы на винде нормально билдилась дллка
#ifdef _WIN32
    #define SH_LIB __declspec(dllexport)
#else
    #define SH_LIB
#endif

namespace SS {
    class SH_LIB Session; // Просто обьявление класса

    struct SH_LIB Config { // Хранение данных для сервера
        std::string dbHost;
        std::string dbPort;
        std::string dbName;
        std::string dbUser;
        std::string dbPass;
        unsigned short port;
    };

    enum class SH_LIB IPV { // Выбор версии для хоста
        V4,
        V6
    };

    class SH_LIB Server {
    public:
        Server(IPV ipv, Config conf);

        int run(); // Запуск сервера
    private:
        void startAccept(); // 
    private:
        IPV _ipVersion;
        Config _config;

        boost::asio::io_context _ioContext;
        boost::asio::ip::tcp::acceptor _acceptor;
    };
}