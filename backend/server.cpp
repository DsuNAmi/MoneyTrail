#include "includes/server.hpp"


// helper
std::string read_file_to_string(const std::string & path){
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if(!file){
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); // read all things contains symbol
    return buffer.str();
}

std::string get_mime_type(const std::string & file_path){
    if(file_path.ends_with(".html")) return "text/html";
    if(file_path.ends_with(".css")) return "text/css";
    if(file_path.ends_with(".js")) return "application/javascript";
    if(file_path.ends_with(".json")) return "application/json";
    if(file_path.ends_with(".png")) return "image/png";
    if(file_path.ends_with(".jpg") || file_path.ends_with(".jpeg")) return "image/jpeg";
    //... more

    return "text/plain";
}

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
                if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
                    // Client disconnected normally, do not treat as error
                    _logger.log_file(LogLevel::INFO, "Connection with " + _client_name + " closed by client.");
                } else {
                    _logger.log_file(LogLevel::ERROR, "Read error from " + _client_name + ": " + ec.message());
                }
                _server.remove_closed_sessions(_client_name);
            }
        });
}

void Session::handle_request(const std::string & request){
    std::string method, target, version;
    std::istringstream iss(request);
    iss >> method >> target >> version;

    cerr_h_time(std::string("Request: " + method + " " + target + " " + version).c_str());

    
    std::string body;
    std::string response;
    std::string content_type;

    if(method != "GET"){
        body = "405 Method Not Allowed";
        content_type = "text/plain";
        response = 
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Content-Type: " + content_type + "\r\n"
            "Connection: keep-alive\r\n"
            "\r\n" + body;
        do_write(response);
        return;
    }



    if(target == "/"){
        // Example: serve a simple HTML page
        body = read_file_to_string("../dist/index.html");
        content_type = "text/html";
    }else if(target == "/cpp"){
        nlohmann::json data;
        data["message"] = "Hello from C++ backend!";
        body = data.dump();
        content_type = "application/json";
    }else{
        std::string file_path = "../dist" + target;
        body = read_file_to_string(file_path);
        content_type = get_mime_type(file_path);
    }

    if(body.empty() && target != "/"){
        std::string not_found_body = "404 Not Found";
        response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: " + std::to_string(not_found_body.size()) + "\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: keep-alive\r\n\r\n" + not_found_body; 
    }else{
        response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Content-Type: " + content_type + "\r\n"
            "Connection: keep-alive\r\n\r\n" + body;
    }

    do_write(response);
}

void Session::do_write(const std::string & response){
    auto self(shared_from_this());
    boost::asio::async_write(_socket, boost::asio::buffer(response),
        [this, self](boost::system::error_code ec, std::size_t bytes){
            if(!ec){
                // Continue reading for more requests (keep-alive)
                do_read();
            }else{
                _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                _server.remove_closed_sessions(_client_name);
                _logger.log_file(LogLevel::INFO, "Connection with " + _client_name + " closed.");
            }
        });
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