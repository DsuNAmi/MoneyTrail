/* program's enterence */

#include "includes/config.hpp"
#include "includes/logger.hpp"
#include "includes/server.hpp"
#include "includes/threadpool.hpp"


std::unique_ptr<Logger> load_logger(const Config & config){
    nlohmann::json logger_config = config.get_object(CType::LOGGER);
    if(logger_config["config_name"].get<std::string>() == "logger"){
        std::string log_file_path = logger_config["path"].get<std::string>();
        return std::make_unique<Logger>(log_file_path);
    }else{
        cerr_h_time("logger config mismatch.");
        return std::make_unique<Logger>();
    }
}

std::unique_ptr<Server> load_server(const Config & config, Logger & logger){
    nlohmann::json server_config = config.get_object(CType::SERVER);
    if(server_config["config_name"].get<std::string>() == "server"){
        std::string host = server_config["config_content"]["host"].get<std::string>();
        unsigned short port = server_config["config_content"]["port"].get<unsigned short>();
        int thread_number = server_config["config_content"]["thread_number"].get<int>();
        return std::make_unique<Server>(host, port, thread_number, logger);
    }else{
        logger.log_file(LogLevel::ERROR, "server config mismatch.");
        throw std::runtime_error("server config mismatch.");
    }
}

std::unique_ptr<ThreadPool> load_threadpool(const Config & config, Logger & logger){
    nlohmann::json tp_config = config.get_object(CType::THREADPOOL);
    if(tp_config["config_name"].get<std::string>() == "threadpool"){
        int thread_number = tp_config["config_content"]["thread_number"].get<int>();
        int max_thread_number = tp_config["config_content"]["max_number"].get<int>();
        int thread_add_chunk = tp_config["config_content"]["add_chunk"].get<int>();
        return std::make_unique<ThreadPool>(thread_number, logger, max_thread_number, thread_add_chunk);
    }else{
        logger.log_file(LogLevel::ERROR, "threadpool config mismatch.");
        throw std::runtime_error("threadpool config mismatch.");
    }
}


template<typename... Ptrs>
void reset_all(Ptrs&... ptrs){
    (ptrs.reset(), ...);
}

int main(){
    try
    {

        /* init verb */
        // Config configs = Config();
        auto configs = std::make_unique<Config>();


        //load logger
        // Logger logger = load_logger(configs);
        auto logger = load_logger(*configs);

        //load threadpool
        // ThreadPool threadpool = load_threadpool(configs, logger);
        auto threadpool = load_threadpool(*configs, *logger);
        threadpool->start();

        //load database
        
        //load server
        // Server server = load_server(configs, logger);
        auto server = load_server(*configs, *logger);

        threadpool->enqueue([&server](){
            server->start();
        });

        char command;
        cerr_h_time("Enter key 'Q' or 'q' to quit the program.");
        cerr_h_time("Enter key 'R' or 'r' to restart server.");
        cerr_h_time("Enter key 'A' or 'a' to add thread to threadpool.");
        cerr_h_time("Enter key 'C' or 'c' to close the server.");
        while(std::cin >> command){
            if(command == 'q' || command == 'Q'){
                reset_all(server, threadpool, logger);
                break;
            }
            else if(command == 'r' || command == 'R'){
                server.reset();
                server = load_server(*configs, *logger);
                threadpool->enqueue([&server](){
                    server->start();
                });
                logger->log_file(LogLevel::INFO, "Server restarted.");
            }else if(command == 'c' || command == 'C'){
                server.reset();
                logger->log_file(LogLevel::INFO, "Server closed.");
            }else if(command == 'a' || command == 'A'){
                threadpool->extend();
                logger->log_file(LogLevel::INFO, "ThreadPool extended to " + std::to_string(threadpool->size()) + " threads.");
            }else{
                cerr_h_time("Unknown command.");
            }
        }
        cerr_h_time("Program exited normally. All resources cleaned up.");
    }
    catch(const std::exception& e)
    {
        cerr_h_time(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
    
}