#include "includes/config.hpp"



// Config
Config::Config()
: _config_path("../config.json")
{
    load_file();
}

Config::Config(std::string config_path)
: _config_path(std::move(config_path))
{
    load_file();
}


void Config::load_file(){
    try
    {
        std::ifstream infile(_config_path, std::ios::beg);
        infile >> configs;
        if(configs.empty()){
            throw std::ios::failure("config file is empty.");
        }
    }
    catch(const std::exception& e)
    {
        cerr_h_time(e.what());
    }
    
}

nlohmann::json Config::get_object(CType config_type) const{
    return configs[static_cast<int>(config_type)];
}