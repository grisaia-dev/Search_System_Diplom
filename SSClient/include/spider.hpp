#pragma once
#include <string>

namespace SS {
    struct Config {
        std::string dbHost; 
        std::string dbPort; 
        std::string dbName; 
        std::string dbUser; 
        std::string dbPass;
        std::string l_protocol;
        std::string l_host;
        std::string l_start_page;
        int depth;
    };

}
