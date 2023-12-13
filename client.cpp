#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/make_shared.hpp>

using boost::asio::ip::tcp;

class tcp_client
{
public:

    tcp_client(boost::asio::io_service& io,
            const std::string& server, const std::string& nickname)
        : io_(io), server_ip_(server),
        nickname_(nickname), 
        socket_(new tcp::socket(io)), resolver_(io),
        recv_buffer_(new boost::array<char, 256>)
    {
        login();
        start_read();
    }

private:

    void login()
    {
        tcp::resolver::query query(server_ip_, "8080");
        tcp::resolver::results_type endpoints = resolver_.resolve(query);

        socket_->async_connect(tcp::endpoint(boost::asio::ip::make_address(server_ip_), 8080),
            [this](const boost::system::error_code& error)
            {
                if (!error)
                { 
                    std::cout << "Connected!\n";
                    socket_->async_send(boost::asio::buffer(nickname_),
                        [](const boost::system::error_code& error, size_t)
                        {
                            if (!error) std::cout << "Nickname sent!\n";
                        });
                }
            });
    }

    void start_read()
    {
        socket_->async_receive(boost::asio::buffer(*recv_buffer_),
            [this](const boost::system::error_code& error, size_t)
            {
                if (!error) std::cout << recv_buffer_->data() << std::endl;
            });
    }

private:

    boost::asio::io_service& io_;
    std::string server_ip_;
    std::string nickname_;
    boost::shared_ptr<tcp::socket> socket_;
    tcp::resolver resolver_;
    boost::shared_ptr<boost::array<char, 256>> recv_buffer_;
};

int main(int argc, char** argv)
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: ./client <host> <clients_name>" << std::endl;
            return 1;
        }

        boost::asio::io_service io;
        tcp_client client(io, argv[1], argv[2]);
        io.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
