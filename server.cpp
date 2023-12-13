#include <string>
#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/move/move.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::ip::tcp;

struct Client
{
    std::string name_;
    boost::shared_ptr<tcp::socket> socket_;
    boost::array<char, 256> recv_buffer_;

    explicit Client(const std::string& name, 
                    boost::shared_ptr<tcp::socket> socket)
                : name_(name), socket_(socket)
    {
    }
};

std::vector<Client> clients;

class tcp_server
{

public:
    tcp_server(boost::asio::io_service& io)
        : io_(io),
        acceptor_(io, tcp::endpoint(tcp::v4(), 8080))
        , socket_(io)
    {
        start_accept();
    }

private:

    void start_accept()
    {
        acceptor_.async_accept(socket_,
            [this](const boost::system::error_code& error)
            {
                if (!error)
                {
                    std::cout << "Accepted!\n";
                    handle_accept();
                }
            });
    }

    void handle_accept()
    {
        boost::shared_ptr<boost::array<char, 128>> client_nickname(new boost::array<char, 128>);

        socket_.async_receive(boost::asio::buffer(*client_nickname),
            [this, client_nickname](const boost::system::error_code& error, size_t)
            {
                if (!error) 
                {
                    std::cout << client_nickname->data() << std::endl;

                    boost::shared_ptr<std::string> success_message(new std::string("Connected to me!"));

                    socket_.async_send(boost::asio::buffer(*success_message),
                        [](const boost::system::error_code& error, size_t){});
                }
            });

        start_accept();
    }

private:

    boost::asio::io_service& io_;
    tcp::acceptor acceptor_;

    tcp::socket socket_;
};

int main()
{
    try
    {
        boost::asio::io_service io;
        tcp_server server(io);
        io.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
