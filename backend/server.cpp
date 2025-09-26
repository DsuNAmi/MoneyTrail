#include "includes/server.hpp"



//Session
Session::Session(boost::asio::ip::tcp::socket socket, Server & server, Logger & logger)
: _stream(std::move(socket)),
  _server(server),
  _logger(logger),
  _closing(false), _writing(false)
{
    try
    {
        auto & socket_ref = _stream.socket();
        auto remote_endpoint = socket_ref.remote_endpoint();
        _clinet_name = remote_endpoint.address().to_string() + 
                        ":" + std::to_string(remote_endpoint.port());
    }
    catch(const std::exception& e)
    {
        _clinet_name = "unkonwn_client";
        _logger.log_file(LogLevel::WARNING,"Could not get client endpoint: " + std::string(e.what()));
    }
    _logger.log_file(LogLevel::INFO, "Client " + _clinet_name + " has been connected.");
    
}

Session::~Session(){
    if(!_closing){
        stop();
    }
    _logger.log_file(LogLevel::INFO, "Session for " + _clinet_name + " destroyed.");
}

void Session::stop(){
    if(_closing.exchange(true)){
        return; // closing, avoid to close again
    }
    _logger.log_file(LogLevel::INFO, "Closing session for " + _clinet_name);
    do_close();
}



void Session::do_read(){
    if(_closing) return;

    _parser.emplace();
    _parser->body_limit(1024 * 1024);

    auto self(shared_from_this());

    boost::beast::http::async_read(
        _stream, _buffer, *_parser,
        [self](boost::system::error_code ec, std::size_t bytes){
            self->handle_read(ec);
        }
    );
}

void Session::handle_read(boost::system::error_code ec){
    if(ec == boost::beast::http::error::end_of_stream){
        _logger.log_file(LogLevel::INFO, "Client " + _clinet_name + " closed connection gracefully.");
        return stop();
    }
    if(ec){
        if(ec != boost::asio::error::operation_aborted){
            _logger.log_file(LogLevel::ERROR, "Read Error: " + ec.message());
        }
        return stop();
    }

    handle_request(_parser->release());

    if(_parser->keep_alive() && !_closing){
        do_read();
    }else{
        _logger.log_file(LogLevel::INFO, "Closing connection for " + _clinet_name + " (keep_alive=false).");
        stop();
    }
}


void Session::handle_request(net_string && request){
    // boost::beast::http::response<boost::beast::http::string_body> res;
    auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();

    res->version(request.version());
    res->keep_alive(request.keep_alive());

    res->set(http_field::server,"MoneyTrail");
    res->set(http_field::access_control_allow_origin,"*");
    res->set(http_field::access_control_allow_methods,"POST,OPTIONS");
    res->set(http_field::access_control_allow_headers,"Content-Type");


    try
    {
        if(request.method() == http_method::options){
            res->result(http_status::no_content);
            res->body() = "";
        }else if(request.method() == http_method::post && request.target() == "/plan"){
            nlohmann::json json_res;
            try
            {
                if(!request.body().empty()){
                    auto parsed = nlohmann::json::parse(request.body());
                    _logger.log_file(LogLevel::INFO,"Parsed : " + parsed.dump() +  "Form Json has been parsed.");
                    json_res = nlohmann::json::array({
                        {{"name","故宫"},{"cost",100},{"time","3小时"}},
                        {{"name","天安门广场"},{"cost",0},{"time","1小时"}},
                        {{"name","颐和园"},{"cost",50},{"time","2小时"}}
                    });
                }else{
                    json_res = nlohmann::json::array({{{"name","null"},{"cost",0},{"time","null"}}});
                }
            }
            catch(...)
            {
                json_res = nlohmann::json::array({{{"name","null"},{"cost",0},{"time","null"}}});
            }
            res->result(http_status::ok);
            res->set(http_field::content_type,"application/json");
            res->body() = json_res.dump();
        }else{
            res->result(http_status::not_found);
            res->set(http_field::content_type,"text/plain");
            res->body() = "404 Not Found";
        }
    }
    catch(const std::exception& e)
    {
        res->result(http_status::internal_server_error);
        res->body() = std::string("Serer error: ") + e.what();
        res->set(http_field::content_type, "text/plain");
    }
    

    res->prepare_payload();

    _response_queue.emplace(res);
    
    do_write();
}

void Session::do_write(){
    if(_closing || _writing || _response_queue.empty()){
        return;
    }

    _writing = true;

    auto response = _response_queue.front();

    auto self(shared_from_this());

    boost::beast::http::async_write(
        _stream,
        *response,
        [self](boost::system::error_code ec, std::size_t bytes){
            self->handle_write(ec, bytes);
        }
    );
}

void Session::handle_write(boost::system::error_code ec, std::size_t bytes){
    _writing = false;

    if(ec){
        _logger.log_file(LogLevel::ERROR, "wtire error: " + ec.message());
        return do_close();
    }

    _response_queue.pop();
}

void Session::clear_queue(){
    //remove queue
    while(!_response_queue.empty()){
        _response_queue.front().reset();
        _response_queue.pop();
    }
}

void Session::do_close(){
    // _server.remove_client(_clinet_name);

    clear_queue();

    boost::system::error_code ec;

    if(!_stream.socket().is_open()){
        _logger.log_file(LogLevel::WARNING,"Socket " + _clinet_name + " automaticlly disconnected.");
        return;
    }

    _stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    if(ec && ec != boost::asio::error::not_connected){
        _logger.log_file(LogLevel::WARNING, "Socket shutdown warning for " + _clinet_name +  ": " + ec.message());
    }

    _stream.socket().close(ec);
    if(ec){
        _logger.log_file(LogLevel::WARNING, "Socket close warning for " + _clinet_name + ": " + ec.message());
    }

    _closing.store(true);

    _logger.log_file(LogLevel::INFO, "Client " + _clinet_name + " disconnected successfully.");
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
    stop();
    _clients.clear();
    _logger.log_file(LogLevel::INFO, "Server stopped. All things cleaned up.");
}



void Server::stop(){

    boost::system::error_code ec;
    _acceptor.close(ec);
    if(ec){
        _logger.log_file(LogLevel::ERROR, "Error closing acceptor: " + ec.message());
    }
    
    close_all_clients();


    //finally to stop server
    if(!_ioc.stopped()){
        _ioc.stop();
    }

    _logger.log_file(LogLevel::INFO, "Server stopped successfully.");
}

void Server::add_client(const std::shared_ptr<Session> & session){
    auto client_name = session->get_client_name();
    _clients.emplace(client_name, session);
    _logger.log_file(LogLevel::INFO, "Client added: " + client_name + ". Total clients: " + std::to_string(_clients.size()));
}

void Server::remove_client(const std::string & client_name){
    if(_clients.erase(client_name) > 0){
        _logger.log_file(LogLevel::INFO, "client removed: " + client_name + ". Remaining clients: " + std::to_string(_clients.size()));
    }
}

void Server::close_all_clients(){
    _logger.log_file(LogLevel::INFO, "Closing all " + std::to_string(_clients.size()) + " clients...");

    for(auto & [client_name, session] : _clients){
        if(session){
            session.reset();
        }
    }
    // _clients.clear();
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
        _logger.log_file(LogLevel::INFO, "Server started on " + _host + ":" + std::to_string(_port));
        _ioc.run();
    }catch(const std::exception & e){
        _logger.log_file(LogLevel::ERROR, "Exception in start(): " + std::string(e.what()));
        return;
    }
}

void Server::do_accept(){
    if(!_acceptor.is_open()) return;
    _acceptor.async_accept(
        boost::asio::make_strand(_ioc),
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket){
            if(!ec){
                auto session = std::make_shared<Session>(std::move(socket), *this, _logger);
                add_client(session);
                session->start();
                do_accept();
            }else{
                if(ec == boost::asio::error::operation_aborted){
                    _logger.log_file(LogLevel::INFO, "Server acceptor stopped.");
                    return;
                }else{
                    _logger.log_file(LogLevel::ERROR, "Accept error: " + ec.message());
                    do_accept();
                }               
            }
        }
    );
}


