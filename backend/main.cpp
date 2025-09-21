/* program's enterence */

#include "includes/config.hpp"
#include "includes/logger.hpp"
#include "includes/server.hpp"


Logger load_logger(const Config & config){
    nlohmann::json logger_config = config.get_object(CType::LOGGER);
    if(logger_config["config_name"].get<std::string>() == "logger"){
        std::string log_file_path = logger_config["path"].get<std::string>();
        return Logger(log_file_path);
    }else{
        cerr_h_time("logger config mismatch.");
        return Logger();
    }
}

int main(){
    try
    {

        /* init verb */
        Config configs = Config();

        //load logger
        Logger logger = load_logger(configs);
        logger.log_file(LogLevel::INFO, "Logger initiallized.");
        //load database
        
        //load server


    }
    catch(const std::exception& e)
    {
        
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
    
}