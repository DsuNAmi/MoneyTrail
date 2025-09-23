#include "includes/recording_date.hpp"

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