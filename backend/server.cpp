#include "includes/server.hpp"


// Session 
Session::Session(boost::asio::ip::tcp::socket socket, Logger & logger, Server & server)
: _socket(std::move(socket)), _logger(logger), _server(server),
  _client_name(
    _socket.remote_endpoint().address().to_string() 
    + ":" 
    + std::to_string(_socket.remote_endpoint().port())
  )
{
    _logger.log_file(LogLevel::INFO, "New connection from " + _client_name); 
}


void Session::start(){
    _server.add_session(_client_name, shared_from_this());
    do_read();
}


void Session::do_read(){
    auto self(shared_from_this());
    boost::asio::async_read_until(_socket, _buffer, "\r\n\r\n",
        [this, self](boost::system::error_code ec, std::size_t bytes){
            if(!ec){
                std::istream request_stream(&_buffer);
                std::string request;
                std::getline(request_stream, request, '\0');
                _logger.log_file(LogLevel::INFO, "Received request from " + _client_name + ": \n" + request);
                handle_request(request);
            }else{
                _logger.log_file(LogLevel::ERROR, "Read error from " + _client_name + ": " + ec.message());
            }
        });
}

void Session::handle_request(const std::string & request){
    std::string method, target, version;
    std::istringstream iss(request);
    iss >> method >> target >> version;

    std::string body;
    std::string response;

    if(target == "/"){
        body = "Welcom to MoneyTrail Server!";
        response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

    }else{
        body = "404 Not Found";
        response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
    }

    do_write(response);
}

void Session::do_write(const std::string & response){
    auto self(shared_from_this());
    boost::asio::async_write(_socket, boost::asio::buffer(response),
        [this, self](boost::system::error_code ec, std::size_t bytes){
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        });
    _server.remove_closed_sessions(_client_name);
    _logger.log_file(LogLevel::INFO, "Connection with " + _client_name + " closed.");
}

// Server
Server::Server(std::string host, unsigned short port, int thread_number, Logger & logger)
: _host(std::move(host)), _port(port), _thread_number(thread_number),
  _ioc(_thread_number), _acceptor(_ioc, make_endpoint()),
  _logger(logger)
{
    _logger.log_file(LogLevel::INFO, "Server initiallized Successfully.");
}

Server::~Server(){
    clear_sessions();
    stop();
    _logger.log_file(LogLevel::INFO, "Server stopped. All things cleaned up.");
}


void Server::clear_sessions(){
    _sessions.clear();
}

void Server::stop(){
    boost::system::error_code ec;
    _acceptor.close(ec);
    if(ec){
        _logger.log_file(LogLevel::ERROR, "Error closing acceptor: " + ec.message());
    }
    _ioc.stop();
}


boost::asio::ip::tcp::endpoint Server::make_endpoint(){
    if(_host == "localhost" || _host.empty()){
        return boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _port);
    }else{
        return boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(_host), _port);
    }
}



void Server::start(){
    try{
        do_accept();
        _ioc.run();
        _logger.log_file(LogLevel::INFO, "Server started on " + _host + ":" + std::to_string(_port));
    }catch(const std::exception & e){
        _logger.log_file(LogLevel::ERROR, "Exception in start(): " + std::string(e.what()));
        return;
    }
}

void Server::do_accept(){
    _acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket){
            if(!ec){
                auto session = std::make_shared<Session>(std::move(socket), _logger, *this);
                session->start();
                do_accept();
            }else{
                if(ec == boost::asio::error::operation_aborted){
                    _logger.log_file(LogLevel::INFO, "Server acceptor stopped.");
                }else{
                    _logger.log_file(LogLevel::ERROR, "Accept error: " + ec.message());
                }
            }
        });
}