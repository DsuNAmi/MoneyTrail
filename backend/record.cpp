#include "includes/record.hpp"


// Record
Record::Record()
: _price(.0), _life_type(LifeType::UNKNOWN), _in_or_ex(INOREX::UNKNOWN)
{

}


Record::Record(const nlohmann::json & record_json){
    try
    {
        set_title(record_json["title"].get<std::string>());
        set_price(record_json["price"].get<float>());
        _life_type = static_cast<LifeType>(record_json["life_type"].get<int>());
        set_location(record_json["location"].get<std::string>());
        set_record_date(record_json["record_date"].get<std::string>());
        set_remark(record_json["remark"].get<std::string>());
        _in_or_ex = static_cast<INOREX>(record_json["inorex"].get<int>());
    }
    catch(const std::exception& e)
    {
        throw std::invalid_argument("Record arguments are not right.");
    }
    
}