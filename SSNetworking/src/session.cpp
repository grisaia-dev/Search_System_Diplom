#include <SSNetworking/session.hpp>
#include <SSHelp/hit.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/impl/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/detail/error_code.hpp>
#include <iostream>

namespace SS {
    Session::Session(boost::asio::io_context& ioContext) : _socket(ioContext) {
        _message = "����������� �����������!";
    }

    void Session::start() {
        auto self = shared_from_this();

        boost::asio::async_write(_socket, boost::asio::buffer(_message), 
            [self](const boost::system::error_code& error, size_t bytesTransferred) {
                if (error) {
                    std::cout << M_ERROR << "������ �������� ���������!" << std::endl;
                } else {
                    std::cout << M_HIT << "����������� " << bytesTransferred << " ���� ������!" << std::endl;
                }
            });

        boost::asio::streambuf buffer;
        _socket.async_receive(buffer.prepare(512),
            [this](const boost::system::error_code& error, size_t bytesTransferred) {
                if (error == boost::asio::error::eof) {
                    std::cout << M_HIT << "������ ����������!" << std::endl;
                } else if (error) {
                    std::cout << M_ERROR << "������ ���������� �������������!" << std::endl;
                }
            });
    }
}
