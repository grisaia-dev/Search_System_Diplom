#include <SSDataBase/db.hpp>
#include <SSHelp/hit.hpp>
#include <pqxx/transaction>

namespace SS {
    db::db() {}
    db::~db() {
        if (m_connection != nullptr) {
            if (m_connection->is_open()) {
                m_connection->close(); 
                std::cout << M_HIT << "Disconnecting from the database!" << std::endl;
            }
            delete m_connection;
        } 
    }

    void db::connect() {
        std::cout << M_HIT << "Connecting to data base..." << std::endl;
        try {
            try {
                m_connection = new pqxx::connection(s_connection.c_str());
            } catch (const pqxx::broken_connection) {}
            if (m_connection != nullptr) {
                if (m_connection->is_open())
                    std::cout << M_GOOD << "Connection completed!" << std::endl;
            } else {
                throw Exception_notValid("It is impossible to connect to the database, check that the connection data is correct! Or service pg_ctl!");
            }
        } catch (const Exception_notValid::broken_connection& ex) {
            std::cout << M_ERROR << ex.what() << std::endl;
        }
    }

    void db::set_connect_string(const std::string& host, const std::string& port, 
        const std::string& name, const std::string& user, const std::string& password) {
        s_connection = "host=" + host + " " +
                       "port=" + port + " " + 
                       "dbname=" + name + " " +
                       "user=" + user + " " +
                       "password=" + password;
    }

    void db::create_structure() {
        if (m_connection != nullptr) {
            pqxx::work tx(*m_connection);
            //tx.exec(tx.esc("CREATE SCHEMA IF NOT EXISTS database;"));
            tx.exec(tx.esc("CREATE TABLE IF NOT EXISTS Documents (id SERIAL PRIMARY KEY, "
                    "protocol VARCHAR(100) NOT NULL, "
                    "hostName VARCHAR(250) NOT NULL, "
                    "query VARCHAR(250) NOT NULL);"));
            tx.exec(tx.esc("CREATE TABLE IF NOT EXISTS Words (id SERIAL PRIMARY KEY, "
                    "word VARCHAR(33) NOT NULL);"));
            tx.exec(tx.esc("CREATE TABLE IF NOT EXISTS DocumentsWords (docLink_id integer NOT NULL REFERENCES Documents (id), "
                    "word_id integer NOT NULL REFERENCES Words (id), "
                    "count integer NOT NULL);"));

            tx.commit();
            std::cout << M_GOOD << "The structure of database created!" << std::endl;
        }
    }

    void db::insert_data(const std::map<std::string, int> &words, const Link &link) {
        pqxx::work tx(*m_connection);
        std::string query = "INSERT INTO Documents VALUES ( nextval('documents_id_seq'::regclass), "
		                    "'" + link.protocol + "', '" + link.host + "', '" + link.query + "') RETURNING id";

        int id_document = tx.query_value<int>(query);
        int id_word = 0;
        for (const auto element : words) {
            query = "SELECT id FROM Words WHERE word='" + element.first + "'";
            id_word = tx.query_value<int>(query);
            if (id_word == 0) {
			    query = "INSERT INTO Words VALUES ( nextval('words_id_seq'::regclass), '" + element.first + "') RETURNING id";
			    id_word = tx.query_value<int>(query);
		    }
            query = "INSERT INTO DocumentsWords(docLink_id, word_id, count) "
				"VALUES (" + std::to_string(id_document) + ", " + std::to_string(id_word) + ", " + std::to_string(element.second) + ") ";
            tx.exec(tx.esc(query));
        }
        tx.commit();
    }
}