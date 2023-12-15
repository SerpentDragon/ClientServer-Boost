#include <string>
#include <thread>
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
        : io_(io), server_ip_(server), nickname_(nickname), 
        socket_(io)
    {
        recv_buffer_.fill(0);
        login();
    }

    ~tcp_client()
    {
        write_thread_.join();
    }

private:

    void login()
    {
        tcp::resolver resolver(io_);
        tcp::resolver::query query(server_ip_, "8080");
        tcp::resolver::results_type endpoints = resolver.resolve(query);

        socket_.async_connect(tcp::endpoint(boost::asio::ip::make_address(server_ip_), 8080),
            [this](const boost::system::error_code& error)
            {
                if (!error)
                { 
                    socket_.async_send(boost::asio::buffer(nickname_),
                        [this](const boost::system::error_code& error, size_t)
                        {
                            if (!error) 
                            {
                                write_thread_ = std::thread(std::bind(&tcp_client::start_write, this));
                                start_read();     
                            }
                        });
                }
                else std::cerr << "Error connecting to server: " << error.message() << std::endl;
            });
    }

    void start_read()
    {
        socket_.async_receive(boost::asio::buffer(recv_buffer_),
            [this](const boost::system::error_code& error, size_t)
            {
                if (!error) 
                {
                    std::cout << recv_buffer_.data() << std::endl;
                    recv_buffer_.fill(0);
                }
                else std::cerr << "Error during reading: " << error.message() << std::endl;

                start_read();
            });
    }

    void start_write()
    {
        boost::system::error_code ignored_error;

        while(true)
        {
            std::string message;
            std::getline(std::cin, message);

            if (message == "#") break;

            socket_.send(boost::asio::buffer(message), 0, ignored_error);
        }
    }

private:

    boost::asio::io_service& io_;

    std::string server_ip_;
    std::string nickname_;

    tcp::socket socket_;

    boost::array<char, 256> recv_buffer_;

    std::thread write_thread_;
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
