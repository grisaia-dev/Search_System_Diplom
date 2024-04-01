#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <filesystem>

namespace INIP {
	class Parser {
	public:
		Parser(const std::filesystem::path filePath);
		~Parser();

		template<typename T>
		T get_value(const std::string& section_key, const T& defaultValue = T());
		template<>
		std::string get_value(const std::string& section_key, const std::string& defaultValue);
	private:
		enum class State {
			ReadyForData,
			Comment,
			Section,
			Key,
			KeyDone,
			Equal,
			Variable
		};

	private: // Functions
		void ProcessRead(const std::string_view& str);
		void Reset();
		void Pair(const std::string& section, const std::string& key, const std::string& value);
		void create_file_config();

	private: // Exceptions
		class Exception_runtime : public std::runtime_error {
		public:
			using std::runtime_error::runtime_error;
		};

		class Exception_notValid : public std::invalid_argument {
		public:
			using std::invalid_argument::invalid_argument;
		};

	private: // Variables
		State m_state = State::ReadyForData;
		size_t m_line = 1;
		std::string m_currentSection, m_currentKey, m_currentValue;
		std::map<std::string, std::string> m_sKeyValue;

		std::ifstream file_ini;

	};
	// end Parser
} // end INIP