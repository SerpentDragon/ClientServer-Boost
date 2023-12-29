#include <map>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

using boost::asio::ip::tcp;
namespace po = boost::program_options;

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

std::map<size_t, Client> clients;

std::string participants()
{
    std::string list_of_clients = "List of Clients:\n";
    for(const auto& client : clients) 
        list_of_clients += client.second.name_ + "\n";

    return list_of_clients;
}

size_t make_id()
{
    if (clients.empty()) return 0;
    else return clients.rbegin()->first + 1;
}

class tcp_server
{

public:
    tcp_server(boost::asio::io_service& io, const int port)
        : io_(io), port_(port),
        acceptor_(io, tcp::endpoint(tcp::v4(), port))
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
        size_t new_id = make_id();
        clients.insert({new_id, Client(new_socket, nickname)});

        std::string welcome_message = "Successfuly connected to server. Start messaging!";
        std::string join_message = nickname + " joined the chat!\n";
        boost::system::error_code ignored_error;

        new_socket->send(boost::asio::buffer(welcome_message), 0, ignored_error);        
        for(const auto& client : clients)
        {
            if (client.first != new_id)
                client.second.socket_->send(boost::asio::buffer(join_message), 0, ignored_error);
        }

        start_read(new_id);
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
                    if (error == boost::asio::error::connection_refused || 
                        error == boost::asio::error::eof) logout_client(index);
                    else if (error)
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
    int port_;
};

int main(int argc, char** argv)
{
    po::options_description desc("Allowed options");

    try
    {
        desc.add_options()
            ("help", "produce help message")
            ("port,p", po::value<int>()->required(), "port to run on : required");
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        boost::asio::io_service io;
        tcp_server server(io, vm["port"].as<int>());
        io.run();
    }
    catch(const po::required_option& ex)
    {
        std::cerr << "Error: the '" << ex.get_option_name() << 
            "' parameter is required" << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
