#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <thread>
#include <functional>
#include <mutex>
#include <sstream>
#include <fstream>
#include <optional>
#include <queue>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "logger.hpp"


using net_string = boost::beast::http::request<boost::beast::http::string_body>;
using http_method = boost::beast::http::verb;
using http_status = boost::beast::http::status;
using http_field = boost::beast::http::field;


class Server;

class Session : public std::enable_shared_from_this<Session>{
    public:
        Session(boost::asio::ip::tcp::socket socket, Server & server, Logger & logger);
        virtual ~Session();

        void start() {do_read();}
        void stop();

        std::string get_client_name() const {return _clinet_name;}


    private:
        void do_read();
        void handle_read(boost::system::error_code ec);
        void handle_request(net_string && request);

        void do_write();
        void handle_write(boost::system::error_code ec, std::size_t bytes);

        void do_close();
        void clear_queue();



    private:
        boost::beast::tcp_stream _stream;
        boost::beast::flat_buffer _buffer;
        
        
        
        std::string _clinet_name;
        std::atomic<bool> _closing;
        
        std::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> _parser;

        Server & _server;
        Logger & _logger;

        std::queue<std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>>> _response_queue;
        std::atomic<bool> _writing;
};


class Server{
    public:
        Server(std::string host, unsigned short port, int thread_number, Logger & logger);
        virtual ~Server();
    
        Server(const Server &) = delete;
        Server & operator=(const Server &) = delete;
        
        
        void start();
        void stop();

    public:

        void add_client(const std::shared_ptr<Session> & session);
        void remove_client(const std::string & client_name);
        void close_all_clients();

        
        
    private:

        void do_accept();

        boost::asio::ip::tcp::endpoint make_endpoint();



    private:
        std::string _host;
        unsigned short _port;
        int _thread_number;
        boost::asio::io_context _ioc;
        boost::asio::ip::tcp::acceptor _acceptor;



    private:
        std::unordered_map<std::string, std::shared_ptr<Session>> _clients;
        Logger & _logger;
};