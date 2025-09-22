#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <mutex>
#include <memory>
#include <fstream>
#include <concepts>
#include <ctime>


template<typename T> concept loggerable =
    std::is_same_v<T, std::string> || 
    std::is_same_v<T, const char*>;


enum class LogLevel{
    INFO = 1,
    WARNING,
    ERROR,
    LOGGER,
    SUCCESS,
};

class Logger{
    public:
        // force to const char * and std::string
        explicit Logger() = default;
        Logger(std::string log_file_path);
        virtual ~Logger();


        Logger(const Logger &) = delete;
        Logger & operator=(const Logger &) = delete;


        template<loggerable LOG>
        void log_cout(LogLevel ll, LOG message){
            log(std::cout, ll, message);
        }

        template<loggerable LOG>
        void log_file(LogLevel ll, LOG message){
            if(_log_file_path.empty()){
                log_cout(ll, message);
            }else{
                std::lock_guard<std::mutex> lock(mutex);
                log(_file, ll, message);
            }
        }



    private:
        template<loggerable LOG>
        void log(std::ostream & os, LogLevel ll,  LOG messages){
            os << "{" + log_to_string(ll) + "}-[" + cur_time() + "]::" << messages <<
            std::endl;
        }

        std::string cur_time(){
            std::time_t time(std::time(nullptr));
            char buff[32];
            std::strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
            return std::string(buff);
        }

        std::string log_to_string(LogLevel ll){
            switch (ll)
            {
                case LogLevel::INFO: return "INFO";
                case LogLevel::WARNING: return "WARNING";
                case LogLevel::ERROR: return "ERROR";
                case LogLevel::LOGGER: return "LOGGER";
                case LogLevel::SUCCESS: return "SUCCESS";
                default: return "UNKNOWN";
            }
        }

    private:
        void load_file();


    private:
        std::string _log_file_path;
        std::ofstream _file;
        std::mutex mutex;      
};


// helper
void cerr_h_time(const char * em);
