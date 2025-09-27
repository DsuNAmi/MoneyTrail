#include "includes/recommands.hpp"



//helper
std::string jet_to_string(JsonErrorType jet){
    if(jet == JsonErrorType::SUCCESS){
        return "";
    }
    switch (jet)
    {
        case JsonErrorType::INVALID : return "DATA INVAILD";
        case JsonErrorType::NOTFIT : return "DATA NOTFIT";
        default: return "OTHER";
    }
}

namespace Recommands{

    //Recommander;
    Recommander::Recommander(const json & send_json, Logger & logger)
    : _logger(logger)
    {
        logger.log_file(LogLevel::INFO, "Parse Send Json....");
        parsed(send_json);
        JsonErrorType jet = check_data();
        if(JsonErrorType::SUCCESS == jet){
            logger.log_file(LogLevel::INFO, "Parsed Json Successfully.");
        }else{
            throw std::invalid_argument(jet_to_string(jet));
        }
    }


    void Recommander::parsed(const json & parsed_json){
        try
        {
            _destination = parsed_json.value("destination", "");
            _travel_days = parsed_json.value("days", 0);
            _travel_budgets = parsed_json.value("budgets",0.0);
        }
        catch(const std::exception& e)
        {
            _logger.log_file(LogLevel::ERROR, "Parsed Failed: " + std::string(e.what()));
            return;
        }
        
    }


    JsonErrorType Recommander::check_data(){
        if(_destination.empty() || 0 == _travel_days || 0.0 == _travel_budgets){
            _logger.log_file(LogLevel::WARNING, "Parsed Failed.");
            return JsonErrorType::INVALID;
        }


        if(_destination.size() > 50 || 0 < _travel_days || 0.0 > _travel_budgets){
            _logger.log_file(LogLevel::WARNING, "Data is not fit.");
            return JsonErrorType::NOTFIT;
        }

        //....


        return JsonErrorType::SUCCESS;
    }


    json Recommander::recommand_algorithm(){
        json res_json;
        
    }
}