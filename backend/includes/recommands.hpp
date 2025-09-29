#pragma once

#include <nlohmann/json.hpp>
#include <exception>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <functional>
#include <vector>
#include <unordered_map>



#include "logger.hpp"
#include "threadpool.hpp"


enum class JsonErrorType{
    SUCCESS = 0,
    INVALID = 1,
    NOTFIT,
    //....
};

enum class ModelType{
    NONE = 0,
    QWEN = 1
};

enum class PlatformType{
    NONE = 0,
    OLLama = 1
};

//helper
std::string jet_to_string(JsonErrorType jet);


namespace Recommands{
    using json = nlohmann::json;


    class AIModel{
        public:
            AIModel(PlatformType platform_type, ModelType model_type, Logger & logger);
            virtual ~AIModel() = default;

            std::string query_ai(const std::string & prompt);
            
            json str_to_json(const std::string & answer);


        private:

            std::string query_ollama_qwen(const std::string & prompt);
            json combine_ollama_qwen(const std::string & prompt);

            std::string combine_key(){return std::to_string(static_cast<int>(_platform_type)) + "_" + std::to_string(static_cast<int>(_model_type));}

        private:

            PlatformType _platform_type;
            ModelType _model_type;

            std::string _host;
            unsigned int _port;

            boost::asio::io_context _ioc;
            boost::asio::ip::tcp::resolver _resolver;
            boost::beast::tcp_stream _stream;

            Logger & _logger;

    };

    class Recommander{
        public:
            Recommander(const json & send_json, Logger & logger);
            virtual ~Recommander() = default;


            Recommander(const Recommander &) = delete;
            Recommander & operator=(const Recommander &) = delete;

        public:
            virtual json recommand_algorithm();

        private:
            JsonErrorType check_data();
            void parsed(const json & parsed_json);
            std::string get_prompt();


        private:
            std::string _location;
            int _travel_days;
            float _travel_budget;

        private:
            Logger & _logger;

    };


    // class AI_Recommander : public Recommander{

    // };


    // Recommand Tools
}

