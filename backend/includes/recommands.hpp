#pragma once

#include <nlohmann/json.hpp>
#include <exception>
#include <boost/asio.hpp>
#include <boost/beast.hpp>



#include "logger.hpp"


enum class JsonErrorType{
    SUCCESS = 0,
    INVALID = 1,
    NOTFIT,
    //....
};

//helper
std::string jet_to_string(JsonErrorType jet);


namespace Recommands{
    using json = nlohmann::json;


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
        private:
            std::string _destination;
            int _travel_days;
            float _travel_budgets;

        private:
            Logger & _logger;

    };


    class AI_Recommander : public Recommander{

    };


    // Recommand Tools 
}

