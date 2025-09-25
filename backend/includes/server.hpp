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

        std::string get_client_name() const {return _clinet_name;}


    private:
        void do_read();
        void handle_read(boost::system::error_code ec);
        void do_close();

    private:
        boost::asio::ip::tcp::socket _socket;
        boost::beast::tcp_stream _stream;
        boost::beast::flat_buffer _buffer;


        std::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> _parser;

        std::string _clinet_name;


        Server & _server;
        Logger & _logger;
        

};


class Server{
    public:
        Server(std::string host, unsigned short port, int thread_number, Logger & logger);

        void start();

        void public_hanlde_requet(
            net_string && request,
            boost::beast::tcp_stream & stream
        ){
            handle_request(std::move(request), stream);
        }

        virtual ~Server();
    
        Server(const Server &) = delete;
        Server & operator=(const Server &) = delete;

    
    
    private:

        void do_accept();

        void handle_request(
            net_string && request,
            boost::beast::tcp_stream & stream
        );

        boost::asio::ip::tcp::endpoint make_endpoint();


        void stop();

    private:
        std::string _host;
        unsigned short _port;
        int _thread_number;
        boost::asio::io_context _ioc;
        boost::asio::ip::tcp::acceptor _acceptor;


    private:
        std::unordered_map<std::string, std::shared_ptr<Session>> _clinets;
        Logger & _logger;
        

};