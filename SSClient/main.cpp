#include <iostream>
#include <boost/asio.hpp>
#include <SSHelp/hit.hpp>
#include <SSParserConfig/config_parser.hpp>

using boost::asio::ip::tcp;

int main(void) {
	setlocale(LC_ALL, "ru");
	INIP::Parser par("config.ini");
	try {
		boost::asio::io_context	ioContext;
		tcp::resolver resolver(ioContext);

		auto endpoint = resolver.resolve("127.0.0.1", "1337");

		tcp::socket socket(ioContext);
		boost::asio::connect(socket, endpoint);


		while (true) {
			std::array<char, 128> buff;
			boost::system::error_code error;

			size_t len = socket.read_some(boost::asio::buffer(buff), error);
			if (error == boost::asio::error::eof) {
				break;
			} else if (error) {
				throw boost::system::system_error(error);
			}

			std::cout << M_ENTER; std::cout.write(buff.data(), len); std::cout << std::endl;
		}
	} catch (const std::exception& ex) {
		std::cerr << M_ERROR << ex.what() << std::endl;
	}
    return EXIT_SUCCESS;
}
