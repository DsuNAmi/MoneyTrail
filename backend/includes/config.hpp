#pragma once

#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include <exception>


#include "logger.hpp"

enum class CType{
    SERVER = 0,
    LOGGER,
    DATABASE
    /* ... */
};

class Config{
    public:
        Config();
        Config(std::string config_path);
        virtual ~Config() = default;


        nlohmann::json get_object(CType config_type) const;

    private:
        std::vector<nlohmann::json> _configs;

        void load_file();

        //ctype
        nlohmann::json configs;


        // static constexpr size_t _init_capacity = 5;
        std::string _config_path;
};