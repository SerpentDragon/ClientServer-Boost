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

    explicit Client(boost::shared_ptr<tcp::socket> socket,
        const std::string& name = "")
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
    {
        start_accept();
    }

private:

    void start_accept()
    {
        boost::shared_ptr<tcp::socket> new_socket(new tcp::socket(io_));

        acceptor_.async_accept(*new_socket,
            [this, new_socket](const boost::system::error_code& error)
            {
                if (!error) handle_accept(new_socket);
                else std::cout << "Error during accepting: " << error.message() << std::endl;
            });
    }

    void handle_accept(boost::shared_ptr<tcp::socket> new_socket)
    {
        boost::shared_ptr<boost::array<char, 128>> client_nickname(new boost::array<char, 128>);

        new_socket->async_receive(boost::asio::buffer(*client_nickname),
            [this, client_nickname, new_socket](const boost::system::error_code& error, size_t)
            {
                if (!error) login_client(new_socket, client_nickname->data());
                else  std::cerr << "Error during getting a nickname: " << error.message() << std::endl;
            });

        start_accept();
    }

    void login_client(boost::shared_ptr<tcp::socket> new_socket, const std::string& nickname)
    {
        clients.emplace_back(Client(new_socket, nickname));

        boost::shared_ptr<std::string> success_message(
                new std::string("Successfuly connected to server. Start messaging!"));
        boost::system::error_code ignored_error;

        clients[clients.size() - 1].socket_->send(boost::asio::buffer(*success_message), 0, ignored_error);
    }

    void start_read(size_t index)
    {
 
    }


private:

    boost::asio::io_service& io_;
    tcp::acceptor acceptor_;
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
