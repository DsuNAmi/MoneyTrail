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
    if(file_path.ends_with(".svg")) return "image/svg+xml";
    if(file_path.ends_with(".ico")) return "image/x-icon";
    if(file_path.ends_with(".woff")) return "font/woff";
    if(file_path.ends_with(".woff2")) return "font/woff2";
    if(file_path.ends_with(".ttf")) return "font/ttf";
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
    _logger.log_file(LogLevel::ERROR, "start");
}

void Session::do_read(){
    auto self(shared_from_this());
    auto buffer = std::make_shared<boost::beast::flat_buffer>();
    auto request = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();

    boost::beast::http::async_read(_socket, *buffer, *request,
        [this, self, buffer, request](boost::beast::error_code ec, std::size_t bytes){
            if(ec){
                handle_disconnection(ec);
                return;
            }

            _logger.log_file(LogLevel::ERROR, "do_read");
            handle_request(request,buffer);
        });
}

void Session::handle_request(std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> request,
                             std::shared_ptr<boost::beast::flat_buffer> buffer){
    boost::beast::http::response<boost::beast::http::string_body> response;
    response.version(request->version());
    response.keep_alive(request->keep_alive());
    response.set(boost::beast::http::field::server,"MoneyTrail");
    response.set(boost::beast::http::field::access_control_allow_origin,"*");
    response.set(boost::beast::http::field::access_control_allow_methods,"POST, OPTIONS");
    response.set(boost::beast::http::field::access_control_allow_headers,"Content-Type");
    
    _logger.log_file(LogLevel::ERROR, "load_data");


    try
    {
        if(request->method() == boost::beast::http::verb::options){
            response.result(boost::beast::http::status::no_content);
            response.body() = "";
        }else if(request->method() == boost::beast::http::verb::post && request->target() == "/plan"){
            nlohmann::json json_response;
            try
            {
                if(!request->body().empty()){
                    auto parsed = nlohmann::json::parse(request->body());
                    (void)parsed;
                    //continue;
                }
                json_response = nlohmann::json::array({
                    {{"name","故宫"},{"cost",100},{"time","3小时"}},
                    {{"name","天安门广场"},{"cost",0},{"time","1小时"}},
                    {{"name","颐和园"},{"cost",50},{"time","2小时"}}
                });
            }
            catch(...)
            {
                json_response = nlohmann::json::array({{{"name","null"},{"cost",0},{"time","null"}}});
            }
            response.result(boost::beast::http::status::ok);
            response.set(boost::beast::http::field::content_type,"application/json");
            response.body() = json_response.dump();
        }else{
            response.result(boost::beast::http::status::not_found);
            response.set(boost::beast::http::field::content_type,"text/plain");
            response.body() = "404 Not Found";
        }
    }
    catch(const std::exception& e)
    {
        response.result(boost::beast::http::status::internal_server_error);
        response.body() = std::string("Server error: ") + e.what();
        response.set(boost::beast::http::field::content_type,"text/plain");
    }


    _logger.log_file(LogLevel::ERROR, "return data");




    response.prepare_payload();
    do_write(response);
}

void Session::do_write(boost::beast::http::response<boost::beast::http::string_body> response){
    auto self = shared_from_this();
    boost::beast::http::async_write(_socket, response,
        [this, self](boost::beast::error_code ec, std::size_t bytes){
            if(!ec){
                _logger.log_file(LogLevel::ERROR, "do_write");
                do_read();
            }
            else{
                handle_disconnection(ec);
            }
        });
}


 void Session::handle_disconnection(boost::beast::error_code ec){
            if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
                // Client disconnected normally, do not treat as error
                _logger.log_file(LogLevel::INFO, "Connection with " + _client_name + " closed by client.");
            } else {
                _logger.log_file(LogLevel::ERROR, "Read error from " + _client_name + ": " + ec.message());
            }
            _server.remove_closed_sessions(_client_name);
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