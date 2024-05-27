#pragma once

#include <pqxx/connection>
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
        void clear_table(const std::string table);
        void insert_data(const std::map<std::string, int>& words, const Link& link);
        int get_id_word(const std::string word);
        std::map<int, int> get_word_count(const int id_word);
        Link get_link(const int doc_id);
        bool search_link(const Link& link);

        bool is_open() {
            return m_connection != nullptr && m_connection->is_open();
        }

    protected: // Exception
        class Exception_notValid : public pqxx::broken_connection {
    	public:
    		using pqxx::broken_connection::broken_connection;
    	};
    private:
        pqxx::connection* m_connection = nullptr;
        std::string s_connection;
    };
}
