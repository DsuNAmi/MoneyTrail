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
    _server.add_session(_client_name, shared_from_this());
    _logger.log_file(LogLevel::INFO, "New connection from " + _client_name); 
}


void Session::start(){
    do_read();
}


void Session::do_read(){
    auto self(shared_from_this());
    boost::asio::async_read_until(_socket, _buffer, "\r\n\r\n",
        [this, self](boost::system::error_code ec, std::size_t bytes){

        });
}



// Server
