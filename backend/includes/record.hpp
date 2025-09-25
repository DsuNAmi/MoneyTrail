// #pragma once


// #include <string>
// #include <ctime>
// #include <nlohmann/json.hpp>


// #include "logger.hpp"
// #include "recording_date.hpp"




// enum class LifeType{
//     UNKNOWN = 0,
//     Game = 1,
//     Food,
//     Traffic,
//     Housing,
//     Electronics,
//     Toy,
//     Fruit,
//     Virtual,
//     Movie,
//     Book
// };


// enum class INOREX{
//     UNKNOWN = 0,
//     INCOME = 1,
//     EXPEND,
// };





// class Record{
//     public:
//         Record();
//         //for test
//         Record(std::string title, float price, int life_type, std::string location,
//             std::string record_date, std::string remark, int inorex){
//                 Record(create_json(title, price, life_type, location, record_date, remark, inorex));
//             }
//         Record(const nlohmann::json & record_json);
//         Record(const std::string & record_string) {
//             Record(str_to_json(record_string));
//         }
//         virtual ~Record() = default;

//     public:
//         static nlohmann::json create_json(std::string title, float price, int life_type, std::string location,
//             std::string record_date, std::string remark, int inorex){
//                 nlohmann::json t;
//                 t["title"] = title;
//                 t["price"] = price;
//                 t["life_type"] = life_type;
//                 t["location"] = location;
//                 t["record_date"] = record_date;
//                 t["remark"] = remark;
//                 t["inorex"] = inorex;
//                 return t;
//             }

//     private:
//         std::string _title;
//         float _price;
//         LifeType _life_type;
//         std::string _location;
//         std::string _record_date;
//         std::string _remark;
//         INOREX _in_or_ex;


//         static constexpr size_t _title_max_size = 50;
//         static constexpr size_t _location_max_size = 100;

    
//     private:
//         void set_title(const std::string & title){
//             //search same name 
//             //size limit
//             if(title.size() > _title_max_size){return;}
//             // unautoriazted word
//             // test_word_in_str()
//             // if(!)
//             // set
//             _title = title;
//         }

//         void set_price(float price){
//             if(0 < price) {return;}
//             _price = price;
//         }

//         void set_location(const std::string & location){
//             // location check, 
//             // [Tianjin]/[Tianjin]/[Jinnan]/[Where]
//             // [Hubei]/[Xiantao]/[Xiantao]/[Where]
//             // Chinese MainLand
//             // check_the_location
//             if(location.size() > _location_max_size){return;}
//             _location = location;
//         }

//         void set_record_date(const std::string & recording_date){
//             //2025/05/04 format
//             RecordingDate t_rdate = RecordingDate(recording_date);
//             _record_date = t_rdate.date("Ymd","cn").value();
//         }


//         void set_remark(const std::string & remark){
//             //just test_in_str()
//             _remark = remark;
//         }

//         std::string get_title() const {return _title;}
//         std::string get_price_ss() const {
//             std::stringstream ss;
//             ss << std::format("{:.2}", _price);
//             return ss.str();
//         }
//         float get_price() const {return _price;}
//         std::string get_life_type() {return lt_to_string(_life_type);}
//         std::string get_location() const {return _location;}
//         std::string get_date() const {return _record_date;}
//         std::string get_remark() const {return _remark;}
//         std::string get_inorex() {return ie_to_string(_in_or_ex);}
    

//     private:
//         std::string lt_to_string(LifeType lt){
//             switch (lt)
//             {
//                 case LifeType::Game: return "Game";
//                 case LifeType::Food: return "Food";
//                 case LifeType::Traffic: return "Traffic";
//                 case LifeType::Housing: return "Housing";
//                 case LifeType::Electronics: return "Electronics";
//                 case LifeType::Toy: return "Toy";
//                 case LifeType::Fruit: return "Fruit";
//                 case LifeType::Virtual: return "Virtual";
//                 case LifeType::Movie: return "Movie";
//                 case LifeType::Book: return "Book";
//                 default: return "UNKNOWN";
//             }
//         }

//         std::string ie_to_string(INOREX inorex){
//             switch (inorex)
//             {
//             case INOREX::INCOME: return "Income";
//             case INOREX::EXPEND: return "expend";
//             default: return "Unknown";
//             }
//         }

//         nlohmann::json str_to_json(const std::string & record_string){
//             // format 
//             return nlohmann::json::parse(record_string);
//         }
        
// };

