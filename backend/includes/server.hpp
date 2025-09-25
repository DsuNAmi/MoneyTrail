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

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "logger.hpp"

class Server;

class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(boost::asio::ip::tcp::socket socket, Logger & logger, Server & server);
        ~Session() = default;

        void start();

        // std::string get_client_name() const {return _client_name;}
        // bool is_closed() const {return !_socket.is_open();}
    private:

        void do_read();
        void handle_request(std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> request,
                            std::shared_ptr<boost::beast::flat_buffer> buffer);
        void do_write(boost::beast::http::response<boost::beast::http::string_body>  response);
        void handle_disconnection(boost::beast::error_code ec);

    private: 
        boost::asio::ip::tcp::socket _socket;

        std::string _client_name;

        Logger & _logger;
        Server & _server;
};

class Server{
    public:
        Server(std::string host, unsigned short port, int thread_number, Logger & logger);

        void start();
        void end(){
            clear_sessions();
            stop();
        };
        // void restart(){
        //     end();
        //     start();
        // }
        // bool is_running() const {return !_ioc.stopped();}


        virtual ~Server();
    
        Server(const Server &) = delete;
        Server & operator=(const Server &) = delete;

    public:
        void add_session(const std::string & client_name ,std::shared_ptr<Session> session){
            _sessions.emplace(client_name, session);
        }

        void remove_closed_sessions(const std::string & client_name){
            _sessions.erase(client_name);
        };
    
    
    private:

        void do_accept();

        boost::asio::ip::tcp::endpoint make_endpoint();

        void clear_sessions();

        void stop();

    private:
        std::string _host;
        unsigned short _port;
        int _thread_number;
        boost::asio::io_context _ioc;
        boost::asio::ip::tcp::acceptor _acceptor;

        Logger & _logger;
        std::unordered_map<std::string, std::shared_ptr<Session>> _sessions;
        

};


//helper
std::string read_file_to_string(const std::string & path);

std::string get_mime_type(const std::string & file_path);