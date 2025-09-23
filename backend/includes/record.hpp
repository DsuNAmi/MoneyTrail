#pragma once


#include <string>
#include <ctime>


#include <iostream>
#include <string>
#include <regex>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <memory>

class RecordingDate {
private:
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    bool has_time;

    // 验证日期是否有效
    bool isValidDate() const {
        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;
        
        // 检查月份天数
        static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int maxDays = daysInMonth[month - 1];
        
        // 闰年处理
        if (month == 2 && isLeapYear()) {
            maxDays = 29;
        }
        
        return day <= maxDays;
    }
    
    // 验证时间是否有效
    bool isValidTime() const {
        if (!has_time) return true;
        return hour >= 0 && hour < 24 && 
               minute >= 0 && minute < 60 && 
               second >= 0 && second < 60;
    }
    
    // 判断是否为闰年
    bool isLeapYear() const {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }
    
    // 解析日期字符串
    bool parseDateString(const std::string& dateStr) {
        // 支持的格式：YYYY/MM/DD, YYYY-MM-DD, YYYY/MM/DD HH:MM:SS, YYYY-MM-DD HH:MM:SS
        std::regex pattern1(R"((\d{4})[/-](\d{1,2})[/-](\d{1,2}))");
        std::regex pattern2(R"((\d{4})[/-](\d{1,2})[/-](\d{1,2})\s+(\d{1,2}):(\d{1,2}):(\d{1,2}))");
        
        std::smatch matches;
        
        if (std::regex_match(dateStr, matches, pattern2)) {
            // 带时间的格式
            year = std::stoi(matches[1]);
            month = std::stoi(matches[2]);
            day = std::stoi(matches[3]);
            hour = std::stoi(matches[4]);
            minute = std::stoi(matches[5]);
            second = std::stoi(matches[6]);
            has_time = true;
        } else if (std::regex_match(dateStr, matches, pattern1)) {
            // 不带时间的格式
            year = std::stoi(matches[1]);
            month = std::stoi(matches[2]);
            day = std::stoi(matches[3]);
            hour = minute = second = 0;
            has_time = false;
        } else {
            return false;
        }
        
        return isValidDate() && isValidTime();
    }

public:
    // 构造函数
    RecordingDate(const std::string& dateStr) {
        if (!parseDateString(dateStr)) {
            throw std::invalid_argument("Invalid date format: " + dateStr);
        }
    }
    
    // 默认构造函数（当前日期）
    RecordingDate() {
        // 简单实现，实际项目中应该使用<chrono>
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        year = tm.tm_year + 1900;
        month = tm.tm_mon + 1;
        day = tm.tm_mday;
        hour = tm.tm_hour;
        minute = tm.tm_min;
        second = tm.tm_sec;
        has_time = true;
    }
    
    // 格式化输出
    std::optional<std::string> format(const std::string& formatStr, const std::string& locale = "en") const {
        try {
            std::stringstream ss;
            
            for (size_t i = 0; i < formatStr.length(); ++i) {
                if (formatStr[i] == 'Y') {
                    // 年份
                    if (i + 3 < formatStr.length() && 
                        formatStr[i+1] == 'Y' && formatStr[i+2] == 'Y' && formatStr[i+3] == 'Y') {
                        ss << std::setw(4) << std::setfill('0') << year;
                        i += 3;
                    } else {
                        ss << year;
                    }
                } else if (formatStr[i] == 'm') {
                    // 月份（两位数）
                    ss << std::setw(2) << std::setfill('0') << month;
                } else if (formatStr[i] == 'n') {
                    // 月份（无前导零）
                    ss << month;
                } else if (formatStr[i] == 'd') {
                    // 日期（两位数）
                    ss << std::setw(2) << std::setfill('0') << day;
                } else if (formatStr[i] == 'j') {
                    // 日期（无前导零）
                    ss << day;
                } else if (formatStr[i] == 'H') {
                    // 小时（24小时制，两位数）
                    ss << std::setw(2) << std::setfill('0') << hour;
                } else if (formatStr[i] == 'i') {
                    // 分钟（两位数）
                    ss << std::setw(2) << std::setfill('0') << minute;
                } else if (formatStr[i] == 's') {
                    // 秒（两位数）
                    ss << std::setw(2) << std::setfill('0') << second;
                } else if (formatStr[i] == '\\') {
                    // 转义字符
                    if (i + 1 < formatStr.length()) {
                        ss << formatStr[++i];
                    }
                } else {
                    ss << formatStr[i];
                }
            }
            
            std::string result = ss.str();
            
            // 根据locale添加本地化分隔符
            if (locale == "cn") {
                // 中文格式：将数字之间的特定分隔符替换为中文
                if (formatStr.find("Ymd") != std::string::npos) {
                    size_t y_pos = result.find(std::to_string(year));
                    if (y_pos != std::string::npos) {
                        result.insert(y_pos + std::to_string(year).length(), "年");
                        size_t m_pos = result.find(std::to_string(month));
                        if (m_pos != std::string::npos && m_pos > y_pos) {
                            result.insert(m_pos + std::to_string(month).length(), "月");
                            size_t d_pos = result.find(std::to_string(day));
                            if (d_pos != std::string::npos && d_pos > m_pos) {
                                result.insert(d_pos + std::to_string(day).length(), "日");
                            }
                        }
                    }
                }
            }
            
            return result;
            
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }
    
    // 便捷方法：常用格式
    std::optional<std::string> date(const std::string& formatStr, const std::string& locale = "en") const {
        return format(formatStr, locale);
    }
    
    // 获取原始数据
    int getYear() const { return year; }
    int getMonth() const { return month; }
    int getDay() const { return day; }
    int getHour() const { return hour; }
    int getMinute() const { return minute; }
    int getSecond() const { return second; }
    bool hasTime() const { return has_time; }
    
    // 比较操作符
    bool operator==(const RecordingDate& other) const {
        return year == other.year && month == other.month && day == other.day &&
               hour == other.hour && minute == other.minute && second == other.second;
    }
    
    bool operator<(const RecordingDate& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        if (day != other.day) return day < other.day;
        if (hour != other.hour) return hour < other.hour;
        if (minute != other.minute) return minute < other.minute;
        return second < other.second;
    }
};

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


enum class LifeType{
    UNKNOWN = 0,
    Game = 1,
    Food,
    Traffic,
    Housing,
    Electronics,
    Toy,


    
};


class Record{
    public:
        Record();
        virtual ~Record() = default;

    private:
        std::string _title;
        double _price;
        LifeType _life_type;
        std::string _localtion;
        
};