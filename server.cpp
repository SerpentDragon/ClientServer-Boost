#include <string>
#include <iostream>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

using boost::asio::ip::tcp;

struct Client
{
    std::string name_;
    boost::shared_ptr<tcp::socket> socket_;

    explicit Client(boost::shared_ptr<tcp::socket> socket,
        const std::string& name = "")
        : name_(name), socket_(socket)
    {
    }

    Client() = default;
};

std::unordered_map<int, Client> clients;

std::string participants()
{
    std::string list_of_clients = "List of Clients:\n";
    for(const auto& client : clients) 
        list_of_clients += client.second.name_ + "\n";

    return list_of_clients;
}

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
                else std::cerr << "Error during accepting: " << error.message() << std::endl;
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
        size_t last_position = clients.size();
        clients.insert({last_position, Client(new_socket, nickname)});

        std::string welcome_message = "Successfuly connected to server. Start messaging!";
        std::string join_message = nickname + " joined the chat!\n";
        boost::system::error_code ignored_error;

        new_socket->send(boost::asio::buffer(welcome_message), 0, ignored_error);        
        for(size_t i = 0; i < clients.size() - 1; i++)
        {
            clients[i].socket_->send(boost::asio::buffer(join_message), 0, ignored_error);
        }

        start_read(last_position);
    }
    
    void start_read(size_t index)
    {
        auto received_message = boost::make_shared<boost::array<char, 256>>(boost::array<char, 256>());

        clients[index].socket_->async_receive(boost::asio::buffer(*received_message), 
            [this, index, received_message](const boost::system::error_code& error, size_t len)
            {
                if (!error) 
                {
                    std::string message = received_message->data();
                    handle_read(index, message);
                }
                else 
                {
                    if (error == boost::asio::error::connection_refused) logout_client(index);
                    else if (error != boost::asio::error::eof)
                        std::cerr << "Error during message forwarding: " << error.message() << std::endl;
                }
            });
    }

    void handle_read(size_t sender, std::string& message)
    {
        if (message == "#") logout_client(sender);
        else
        {
            if (message == "list!") 
            {
                message = participants();
                clients[sender].socket_->send(boost::asio::buffer(message), 0);
            }
            else 
            {
                message = "From: " + clients[sender].name_ + "\nMessage: " + message + "\n";
                for(const auto& client : clients) 
                {
                    if (client.first != sender)
                        client.second.socket_->send(boost::asio::buffer(message), 0);
                }
            }

            start_read(sender);
        }
    }

    void logout_client(size_t sender)
    {  
        std::string answer = clients[sender].name_ + " left the chat!\n";
        for(const auto& client : clients)
        {
            if (client.first != sender)
                client.second.socket_->send(boost::asio::buffer(answer), 0);
        }  
             
        for(auto it = clients.begin(); it != clients.end(); it++)
        {
            if (it->first == sender)
            {
                clients.erase(it);
                break;
            }
        }   
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
