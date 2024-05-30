#include <SSParserConfig/config_parser.hpp>
#include <SSHelp/hit.hpp>

INIP::Parser::Parser(const std::filesystem::path filePath) {
	file_ini.open(filePath, std::ios::binary | std::ios::in);
	std::string forEx = "[CONFIG_PARSER]:File " + filePath.string() + " not found!";

	try {
		if (!file_ini.is_open())
			throw Exception_runtime(forEx);
	} catch (const std::runtime_error& ex) {
		std::cout << M_ERROR << ex.what() << std::endl;
		std::cout << M_HIT << "[CONFIG_PARSER]:Creating config file.." << std::endl;
		create_file_config();
		file_ini.open(filePath, std::ios::binary | std::ios::in);
	}

	file_ini.seekg(0, std::ios::end);
	size_t fileSize = file_ini.tellg();
	file_ini.seekg(0, std::ios::beg);

	std::string fileData(fileSize, ' ');
	file_ini.read(&fileData[0], fileSize);

	ProcessRead(fileData);
}

INIP::Parser::~Parser() { file_ini.close(); }

void INIP::Parser::ProcessRead(const std::string_view& str) {
	std::cout << M_HIT << "[CONFIG_PARSER]:Reading a configuration file.." << std::endl;
	try {
		Reset();
		for (char c : str) {
			// Cath control keys
			if (std::iscntrl(c)) {
				switch (c) {
				case '\t':
					[[fallthrough]];
				case '\n':
					break;
				default:
					continue;
				}
			} // end if (std::iscntrl(c))

			switch (m_state) {
			case State::ReadyForData:
				if (c == ';') {
					m_state = State::Comment;
				} else if (c == '[') {
					m_currentSection.clear();
					m_state = State::Section;
				} else if (c == ' ')
					continue;
				else if (c == '\t')
					continue;
				else if (c == '\n') {
					m_line += 1;
					continue;
				} else {
					m_currentKey.clear();
					m_currentKey += c;
					m_state = State::Key;
				}
				break;
			case State::Comment:
				if (c == '\n') {
					m_state = State::ReadyForData;
					m_line += 1;
				}
				break;
			case State::Section:
				if (c == ']') {
					m_state = State::ReadyForData;
				} else if (c == '\t')
					throw Exception_runtime("[CONFIG_PARSER]:Tabs are not allowed in section name! At line: " + std::to_string(m_line));
				else if (c == '\n')
					throw Exception_runtime("[CONFIG_PARSER]:Newlines are not allowed in section name! At line: " + std::to_string(m_line));
				else
					m_currentSection += c;
				break;
			case State::Key:
				if (c == ' ')
					m_state = State::KeyDone;
				else if (c == '=')
					m_state = State::Equal;
				else if (c == '\t') {
					//throw Exception("Tabs are not allowed in the key! At line: " + std::to_string(line)); // Если хотим что бы не было табов
					continue;
				} else if (c == '\n') {
					throw Exception_runtime("[CONFIG_PARSER]:Newlines are not allowed in the key! At line: " + std::to_string(m_line));
				} else
					m_currentKey += c;
				break;
			case State::KeyDone:
				if (c == ' ')
					continue;
				else if (c == '\t')
					continue;
				else if (c == '\n')
					throw Exception_runtime("[CONFIG_PARSER]:not found '='! At line: " + std::to_string(m_line));
				else if (c == '=')
					m_state = State::Equal;
				else
					throw Exception_runtime("[CONFIG_PARSER]:Keys are not allowed to have spaces in them! At line: " + std::to_string(m_line));
				break;
			case State::Equal:
				if (c == ' ')
					continue;
				else if (c == '\t')
					continue;
				else if (c == '\n')
					throw Exception_runtime("[CONFIG_PARSER]:Value can't be empty! At line: " + std::to_string(m_line));
				else {
					m_currentValue.clear();
					m_currentValue += c;
					m_state = State::Variable;
				}
				break;
			case State::Variable:
				if (c == '\n') {
					Pair(m_currentSection, m_currentKey, m_currentValue);
					m_line += 1;
					m_state = State::ReadyForData;
				} else if (c == ';') {
					Pair(m_currentSection, m_currentKey, m_currentValue);
					m_state = State::Comment;
				} else {
					m_currentValue += c;
				}
				break;
			} // end switch (m_state)
		} // end for
		std::cout << M_GOOD << "[CONFIG_PARSER]:Reading complete!" << std::endl;
	} catch (const std::runtime_error& ex) {
		std::cout << M_ERROR << ex.what() << std::endl;
		exit(1);
	}
} // end function

void INIP::Parser::Reset() {
	m_currentSection = "";
	m_currentKey = "";
	m_currentValue = "";
	m_state = State::ReadyForData;
}

void INIP::Parser::Pair(const std::string& section, const std::string& key, const std::string& value) {
	std::string temp = section + "." + key;
	if (m_sKeyValue.count(temp))
		m_sKeyValue[temp] = value;
	else if (!m_sKeyValue.count(temp))
		m_sKeyValue.emplace(temp, value);
}

void INIP::Parser::create_file_config() {
	std::ofstream file("config.ini");
	std::cout << M_GOOD << "[CONFIG_PARSER]:File created!" << std::endl;
	std::cout << M_HIT << "[CONFIG_PARSER]:Filling out the file.." << std::endl;
	std::string options = "[DB]\n"
						  "\thost = localhost\n"
						  "\tport = 5432\n"
						  "\tname = postgres\n"
						  "\tuser = postgres\n"
						  "\tpassword = postgres\n\n"
						  "[SITE]\n"
						  "\tprotocol = https://\n"
						  "\thost = en.wikipedia.org\n"
						  "\tpage = /wiki/Main_Page/\n"
						  "\tdepth = 0\n\n"
						  "[SERVER]\n"
						  "\tport = 1337\n";
	file << options;
	file.close();
}
