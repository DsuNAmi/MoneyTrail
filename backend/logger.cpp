#include "includes/logger.hpp"

// Logger
Logger::Logger(std::string log_file_path)
: _log_file_path(std::move(log_file_path))
{}


Logger::~Logger(){
    if(_file.is_open()){
        _file.close();
    }
}

void Logger::load_file(){
    try
    {
        _file.open(_log_file_path, std::ios::app);
        if(!_file.is_open()){
            throw std::ios::failure("log file open failed.");
        }
    }
    catch(const std::exception& e)
    {
        cerr_h_time(e.what());
    }
    
}
