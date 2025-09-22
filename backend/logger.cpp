#include "includes/logger.hpp"


//helper
void cerr_h_time(const char * em){
    std::time_t time(std::time(nullptr));
    char buff[32];
    std::strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    std::cerr << "[" << buff << "]" << em << std::endl;
}

// Logger
Logger::Logger(std::string log_file_path)
: _log_file_path(std::move(log_file_path))
{
    load_file();
    log_file(LogLevel::INFO, "Logger initiallized.");
}


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
