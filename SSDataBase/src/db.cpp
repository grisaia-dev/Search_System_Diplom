#include <SSDataBase/db.hpp>
#include <SSHelp/hit.hpp>
#include <pqxx/pqxx>
#include <iostream>
#include <mutex>
#include <map>
#include <tuple>

std::mutex mtx;

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
            tx.exec(tx.esc("CREATE SCHEMA IF NOT EXISTS database;"));
            tx.exec(tx.esc("CREATE TABLE IF NOT EXISTS database.Documents (id SERIAL PRIMARY KEY, "
                    "protocol VARCHAR(100) NOT NULL, "
                    "hostName VARCHAR(250) NOT NULL, "
                    "query VARCHAR(250) NOT NULL);"));
            tx.exec(tx.esc("CREATE TABLE IF NOT EXISTS database.Words (id SERIAL PRIMARY KEY, "
                    "word VARCHAR(33) NOT NULL);"));
            tx.exec(tx.esc("CREATE TABLE IF NOT EXISTS database.DocumentsWords (docLink_id integer NOT NULL REFERENCES database.Documents (id), "
                    "word_id integer NOT NULL REFERENCES database.Words (id), "
                    "count integer NOT NULL);"));

            tx.commit();
            std::cout << M_GOOD << "The structure of database created!" << std::endl;
        }
    }

    void db::clear_table(const std::string table) {
        pqxx::work tx(*m_connection);
        tx.exec("DELETE FROM database." + tx.esc(table) );
        tx.commit();
    }

    void db::insert_data(const std::map<std::string, int> &words, const Link &link) {
        pqxx::work tx(*m_connection);
        std::string query;

        std::lock_guard<std::mutex> lock(mtx);
        query = "INSERT INTO database.Documents VALUES ( nextval('database.documents_id_seq'::regclass), '"
		        + tx.esc(link.protocol) + "', '" + tx.esc(link.host) + "', '" + tx.esc(link.query) + "') RETURNING id";
        int id_document = tx.query_value<int>(query);
        int count = 0;
        int id_word = 0;
        for (const auto element : words) {
            query = "SELECT count(id) FROM database.Words WHERE word='" + tx.esc(element.first) + "'";
            int words_count = tx.query_value<int>(query);

            if (words_count == 0) {
                if (count != 0) {
                    query = "SELECT id FROM database.Words WHERE word='" + tx.esc(element.first) + "'";
                    id_word = tx.query_value<int>(query);
                } else {
                    query = "INSERT INTO database.Words VALUES ( nextval('database.words_id_seq'::regclass), '" + tx.esc(element.first) + "') RETURNING id";
                    id_word = tx.query_value<int>(query);
                }
                query = "INSERT INTO database.DocumentsWords(docLink_id, word_id, count) "
				    "VALUES (" + tx.esc(std::to_string(id_document)) + ", " + tx.esc(std::to_string(id_word)) + ", " + tx.esc(std::to_string(element.second)) + ") ";
                tx.exec(query);
            } else {}
        }
        tx.commit();
    }

    int db::get_id_word(const std::string word) {
        pqxx::work tx(*m_connection);

        std::string query = "SELECT count(id) FROM database.Words WHERE word='" + tx.esc(word) + "'";
	    int countWord = tx.query_value<int>(query);
	    if (countWord != 0) {
	    	query = "SELECT id FROM database.Words WHERE word='" + tx.esc(word) + "'";
	    	int id_word = tx.query_value<int>(query);
	    	tx.exec(query);
	    	return id_word;
	    } else {
	    	return 0;
	    }
    }

    std::map<int, int> db::get_word_count(const int id_word) {
        pqxx::work tx(*m_connection);
	    std::map<int, int> word_count;

	    for (auto [doc_id, count] :
            tx.query<int, int>(tx.esc("SELECT docLink_id, count FROM database.DocumentsWords WHERE word_id=" + tx.esc(std::to_string(id_word))))){
		        word_count.insert({ doc_id , count });
	        }
	    return word_count;
    }

    Link db::get_link(const int doc_id) {
	    pqxx::work tx(*m_connection);
	    Link lk;
	    for (std::tuple<std::string, std::string, std::string> tpl :
            tx.query<std::string, std::string, std::string>("SELECT protocol, hostname, query FROM database.Documents WHERE id = " + tx.esc(std::to_string(doc_id)))) {
	    	    lk.protocol = std::get<0>(tpl);
	    	    lk.host = std::get<1>(tpl);
	    	    lk.query = std::get<2>(tpl);
	    }
	    return lk;
    }

    bool db::search_link(const Link& link) {
        pqxx::work tx(*m_connection);
	    int count = tx.query_value<int>("SELECT COUNT(*) FROM database.Documents "
			"WHERE protocol='" + tx.esc(link.protocol) + "' AND hostName='" + tx.esc(link.host) + "' AND query='" + tx.esc(link.query) + "'");
	    if (count == 0) {
		    return true;
	    } else {
		    return false;
	    }
    }
}
