#pragma once

#include <nlohmann/json.hpp>


namespace Recommands{
    using json = nlohmann::json;



    // Recommand Tools 
    json recommand_travel_tour(const json & parsed_json);

    bool is_vaild_input(std::string d, int td, f tb);

}

