#include <SSParserConfig/config_parser.hpp>
#include <SSDataBase/db.hpp>
#include "include/spider.hpp"
#include <SSHelp/hit.hpp>
#include <cstdlib>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/json.hpp>
#include <exception>
#include <netdb.h>
#include <openssl/ssl.h>

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace http = boost::beast::http;
using boost::asio::ip::tcp;

std::string get_html_body(const SS::Link& link) {
    std::string result = "\0";
    try {
        asio::io_context io_context;
        if (link.protocol == "https://") {
            // create ssl certificate
            ssl::context ctx(ssl::context::tlsv13_client);
            ctx.set_default_verify_paths();
            beast::ssl_stream<beast::tcp_stream> stream(io_context, ctx);
            stream.set_verify_mode(ssl::verify_none);
            stream.set_verify_callback([](bool preverified, ssl::verify_context& ctx) {
                return true; // Accept any certificate
            });
            // if can't get ssl
            if (!SSL_set_tlsext_host_name(stream.native_handle(), link.host.c_str())) {
                beast::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
                throw beast::system_error{ec};
            }

            // create resolver
            tcp::resolver resolver(io_context);
			get_lowest_layer(stream).connect(resolver.resolve({ link.host, "https" }));
			get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

            // Creating request for site
            http::request<http::string_body> req;
            req.method(http::verb::get);
            req.target(link.query);
            req.set(http::field::host, link.host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.prepare_payload();
            std::cout << req.base() << "\n";

            // creating handshake and past in request
            //stream.shutdown();
            stream.handshake(ssl::stream_base::client);
            http::write(stream, req);

            // create buff and response for site / go request for host / reading response
            beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(stream, buffer, res);
            result = beast::buffers_to_string(res.body().data());

            // close ssl stream
            beast::error_code ec;
            stream.shutdown(ec);
            if (ec == asio::error::eof) { ec = {}; }
            if (ec) { throw beast::system_error{ec}; }
        }
        io_context.stop();
    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << "\n";
    }
    return result;
}

int main() {
    try {
        SS::Config conf;
        INIP::Parser parser("config.ini");

        conf.dbHost = parser.get_value<std::string>("DB.host");
        conf.dbPort = parser.get_value<std::string>("DB.port");
        conf.dbName = parser.get_value<std::string>("DB.name");
        conf.dbUser = parser.get_value<std::string>("DB.user");
        conf.dbPass = parser.get_value<std::string>("DB.password");
        conf.l_protocol = parser.get_value<std::string>("SITE.protocol");
        conf.l_host = parser.get_value<std::string>("SITE.host");
        conf.l_start_page = parser.get_value<std::string>("SITE.page");
        conf.depth = parser.get_value<int>("SITE.depth");

        SS::Link link{conf.l_protocol, conf.l_host, conf.l_start_page};

        std::cout << M_HIT << "Starting parse link..\n";
        std::string body;
        body = get_html_body(link);
        std::cout << body << "\n";


        std::cout << M_GOOD << "Parsing completed!\n";
    } catch (const std::exception& ex) {
        std::cout << M_ERROR << ex.what() << "\n";
    }
    return EXIT_SUCCESS;
}
