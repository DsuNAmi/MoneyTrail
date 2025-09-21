#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>


#include <boost/asio.hpp>


#include "logger.hpp"

class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(boost::asio::ip::tcp::socket socket, Logger & logger, Server & server);
        ~Session() = default;

        void start();
    private:

        void do_read();
        void handle_request(const std::string & request);
        void do_write(const std::string & response);


    private: 
        boost::asio::ip::tcp::socket _socket;
        boost::asio::streambuf _buffer;

        std::string _client_name;

        Logger & _logger;
        Server & _server;
};

class Server{
    public:
        Server(std::string host, unsigned short port, int thread_number);

        void start();
        void add_session(const std::string & client_name ,std::shared_ptr<Session> session){
            _sessions.emplace(client_name, session);
        }

        ~Server() = default;
    private:

        void do_accept();

        boost::asio::ip::tcp::endpoint make_endpoint();


    private:

        std::string _host;
        unsigned short _port;
        int _thread_number;
        boost::asio::io_context _ioc;
        boost::asio::ip::tcp::acceptor _acceptor;

        Logger & _logger;
        std::unordered_map<std::string, std::shared_ptr<Session>> _sessions;

};