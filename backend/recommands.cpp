#include "includes/recommands.hpp"


namespace Recommands{
    json recommand_travel_tour(const json & parsed_json, Logger & logger){
        //1. read the data
        std::string destination = parsed_json.value("location","");
        int travel_days = parsed_json.value("days", 0);
        float travel_budgets = parsed_json.value("budget",0.0);
        
        
        //2. check the data
        if(!is_vaild_input(destination, travel_days, travel_budgets)){
            logger.log_file(LogLevel::WARNING, "Parsed Data is non-valid.");
            return json::array(); // return null
        }

        //

        

    }

    bool is_vaild_input(const std::string & d, int td, float tb){
        //1. input is non-valid
        if(d.empty() || 0 == td || 0.0 == tb){
            return false;
        }

        //constaint
        if(d.size() > 50 || 0 > td || 0.0 > tb){
            return false;
        }

        //...
    }
    
}