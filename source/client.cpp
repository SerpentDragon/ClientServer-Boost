#include <string>
#include <thread>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

using boost::asio::ip::tcp;
namespace po = boost::program_options;

class tcp_client
{
public:

    tcp_client(boost::asio::io_service& io,
            const std::string& server, const int port,
            const std::string& nickname)
        : io_(io), server_ip_(server), port_(port),
        nickname_(nickname), socket_(io)
    {
        recv_buffer_.fill(0);
        thread_active_ = true;
        login();
    }

    ~tcp_client()
    {
        if (write_thread_.joinable())
            write_thread_.join();
    }

private:

    void login()
    {
        tcp::resolver resolver(io_);
        tcp::resolver::query query(server_ip_, std::to_string(port_));
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
                                write_thread_ = std::thread([this](){ start_write(); });
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
                else if (error == boost::asio::error::eof)
                {
                    std::cerr << "Server connection is broken!\n";
                    thread_active_ = false;                    
                    io_.stop();
                }

                start_read();
            });
    }

    void start_write()
    {
        boost::system::error_code ignored_error;
        std::string message;

        while(thread_active_)
        {
            std::getline(std::cin, message);
            socket_.send(boost::asio::buffer(message), 0, ignored_error);

            if (message == "#")
            {
                io_.stop();
                break;
            }
        }      
    }

private:

    boost::asio::io_service& io_;

    std::string server_ip_;
    int port_;
    std::string nickname_;

    tcp::socket socket_;
    boost::array<char, 256> recv_buffer_;

    std::thread write_thread_;  
    bool thread_active_;
};

int main(int argc, char** argv)
{
    po::options_description desc("Allowed options");

    try
    {
        desc.add_options()
            ("help,h", "produce help message")
            ("host,H", po::value<std::string>()->required(), "host to connect : required")
            ("port,p", po::value<int>()->required(), "port to connect : required")
            ("name,n", po::value<std::string>()->required(), "client's name : required");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        boost::asio::io_service io;
        tcp_client client(io, vm["host"].as<std::string>(), 
            vm["port"].as<int>(), vm["name"].as<std::string>());
        
        io.run();
    }
    catch(const po::required_option& ex)
    {
        std::cerr << "Error: the '" << ex.get_option_name() << 
            "' parameter is required" << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}
