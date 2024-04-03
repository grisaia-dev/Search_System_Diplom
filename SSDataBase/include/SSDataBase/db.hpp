#pragma once

#include <pqxx/connection>
#include <iostream>
#include <string>

#ifdef _WIN32
    #define SH_LIB __declspec(dllexport)
#else
    #define SH_LIB
#endif

namespace SS {
    struct SH_LIB Link {
        std::string protocol;
        std::string host;
        std::string query;
        bool operator=(const Link& link) const {
            return protocol == link.protocol && host == link.host && query == link.query;
        }
    };

    class SH_LIB db {
    public:
        db();
        db(const db&) = delete;
        db& operator=(const db&) = delete;
        ~db();

        void connect();
        void set_connect_string(const std::string& host, const std::string& port, 
            const std::string& name, const std::string& user, const std::string& password);
        void create_structure();
        void insert_data(const std::map<std::string, int>& words, const Link& link);

    private: // Exception
        class Exception_notValid : public pqxx::broken_connection {
    	public:
    		using pqxx::broken_connection::broken_connection;
    	};
    private:
        pqxx::connection* m_connection = nullptr;
        std::string s_connection;
    };
}
