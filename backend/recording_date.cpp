#include "includes/recording_date.hpp"


//RecordingDate
RecordingDate::RecordingDate()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time_t_now);
    _year = tm.tm_year + 1900;
    _month = tm.tm_mon + 1;
    _day = tm.tm_mday;
    _hour = tm.tm_hour;
    _minute = tm.tm_min;
    _second = tm.tm_sec;
    _has_time = true;
}

RecordingDate::RecordingDate(const std::string & date_string){
    if(!parse_date_string(date_string)){
        throw std::invalid_argument("Invalid date format: " + date_string);
    }
}

bool RecordingDate::is_valid_date() const{
    if(_year < 1900 || _year > 2100) return false;
    if(_month < 1 || _month > 12) return false;
    if(_day < 1 || _day > 31) return false;
    
    int max_days = _day_in_months[_month - 1];

    //leap year
    if(_month == 2 && is_leap_year()){
        max_days = 29;
    }

    return _day <= max_days;
}


bool RecordingDate::is_valid_time() const{
    if(!_has_time) return true;
    return _hour > 0 && _hour < 24 &&
           _minute >= 0 && _minute < 60 &&
           _second >= 0 && _second < 60;
}

bool RecordingDate::is_leap_year() const{
    return (_year % 4 == 0 && _year % 100 != 0) || (_year % 400 == 0); 
}

bool RecordingDate::parse_date_string(const std::string& date_string){
    // format : YYYY/MM/DD, YYYY-MM-DD, YYYY/MM/DD HH:MM:SS, YYYY-MM-DD HH:MM:SS
    std::regex pattern1(R"((\d{4})[/-](\d{1,2})[/-](\d{1,2}))");
    std::regex pattern2(R"((\d{4})[/-](\d{1,2})[/-](\d{1,2})\s+(\d{1,2}):(\d{1,2}):(\d{1,2}))");

    std::smatch matches;

    if(std::regex_match(date_string, matches, pattern2)){
        // with time
        _year = std::stoi(matches[1]); // 0 represent full string
        _month = std::stoi(matches[2]);
        _day = std::stoi(matches[3]);
        _hour = std::stoi(matches[4]);
        _minute = std::stoi(matches[5]);
        _second = std::stoi(matches[6]);
        _has_time = true;
    }else if(std::regex_match(date_string, matches, pattern1)){
        _year = std::stoi(matches[1]);
        _month = std::stoi(matches[2]);
        _day = std::stoi(matches[3]);
        _hour = _minute = _second = 0;
        _has_time = false;
    }else{
        return false;
    }


    return is_valid_date() && is_valid_time();
}


std::optional<std::string> RecordingDate::format(const std::string & format_string, const std::string& locale = "en") const{
    try
    {
        std::stringstream ss;

        for(size_t i = 0; i < format_string.size(); ++i){
            if(format_string[i] == 'Y'){
                if(i + 3 < format_string.size() && 
                   format_string[i + 1] == 'Y' && 
                   format_string[i + 2] == 'Y' &&
                   format_string[i + 3] == 'Y'){
                    ss << std::format("{:0^4}",_year);
                    i += 3;
                   }else {ss << _year;}
            }else if(format_string[i] == 'm'){
                ss << std::format("{:0^2}", _month);
            }else if(format_string[i] == 'n'){
                ss << _month;
            }else if(format_string[i] == 'd'){
                ss << std::format("{:0^2}", _day);
            }else if(format_string[i] == 'j'){
                ss << _day;
            }else if(format_string[i] == 'H'){
                ss << std::format("{:0^2}", _hour);
            }else if(format_string[i] == 'i'){
                ss << std::format("{:0^2}", _minute);
            }else if(format_string[i] == 's'){
                ss << std::format("{:0^2}", _second);
            }else if(format_string[i] == '\\'){
                if(i + 1 < format_string.size()){
                    ss << format_string[++i];
                }
            }else{
                ss << format_string[i];
            }
        }


        std::string result = ss.str();

        //locale
        if ("cn" == locale) {
            // 中文格式：将数字之间的特定分隔符替换为中文
            if (format_string.find("Ymd") != std::string::npos) {
                size_t y_pos = result.find(std::to_string(_year));
                if (y_pos != std::string::npos) {
                    result.insert(y_pos + std::to_string(_year).length(), "年");
                    size_t m_pos = result.find(std::to_string(_month));
                    if (m_pos != std::string::npos && m_pos > y_pos) {
                        result.insert(m_pos + std::to_string(_month).length(), "月");
                        size_t d_pos = result.find(std::to_string(_day));
                        if (d_pos != std::string::npos && d_pos > m_pos) {
                            result.insert(d_pos + std::to_string(_day).length(), "日");
                        }
                    }
                }
            }
        } 
        
        return result;

    }
    catch(const std::exception& e)
    {
        return std::nullopt;
    }
    
}


// // 使用示例
// int main() {
//     try {
//         // 测试各种日期格式
//         RecordingDate date1("2025/05/04");
//         RecordingDate date2("2025-05-04 14:30:25");
//         RecordingDate date3("2025/12/25 08:00:00");
        
//         // 测试格式化输出
//         auto result1 = date1.date("Ymd", "cn");
//         auto result2 = date1.date("Ymd", "en");
//         auto result3 = date2.date("Y-m-d H:i:s", "en");
//         auto result4 = date3.date("Y年n月j日", "cn");
        
//         if (result1) std::cout << "中文格式: " << *result1 << std::endl;
//         if (result2) std::cout << "英文格式: " << *result2 << std::endl;
//         if (result3) std::cout << "带时间格式: " << *result3 << std::endl;
//         if (result4) std::cout << "自定义中文: " << *result4 << std::endl;
        
//         // 测试错误格式
//         try {
//             RecordingDate errorDate("2025/13/45"); // 无效日期
//         } catch (const std::exception& e) {
//             std::cout << "正确捕获错误: " << e.what() << std::endl;
//         }
        
//     } catch (const std::exception& e) {
//         std::cerr << "错误: " << e.what() << std::endl;
//     }
    
//     return 0;
// }