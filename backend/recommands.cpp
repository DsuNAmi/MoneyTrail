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


    //AIModel
    AIModel::AIModel(PlatformType platform_type, ModelType model_type, Logger & logger)
    : _ioc(1), _resolver(_ioc), _stream(_ioc),
      _platform_type(platform_type), _model_type(model_type),
      _logger(logger)
    {
        if(_platform_type == PlatformType::OLLama){
            _host = "127.0.0.1";
            _port = 11434;
        }
    }

    json AIModel::combine_ollama_qwen(const std::string & prompt){
        json res_body;
        res_body["model"] = "qwen3:8b";
        res_body["stream"] = false;
        res_body["prompt"] = prompt;
        
        return res_body;
    }



    std::string AIModel::query_ollama_qwen(const std::string & prompt){
        try
        {
            auto const result = _resolver.resolve(_host, std::to_string(_port));
            _stream.connect(result);
    
    
            // std::string body = R"({"model":"qwen3:8b","prompt":")" + prompt + R"("})";
            std::string body = combine_ollama_qwen(prompt).dump();
    
    
            boost::beast::http::request<boost::beast::http::string_body> request(
                boost::beast::http::verb::post, "/api/generate", 11
            );
    
            request.set(boost::beast::http::field::host, _host);
            request.set(boost::beast::http::field::content_type, "application/json");
            request.body() = body;
            request.prepare_payload();
    
            boost::beast::http::write(_stream, request);
    
            boost::beast::flat_buffer buffer;

            boost::beast::http::response<boost::beast::http::string_body> response;
            boost::beast::http::read(_stream, buffer, response);

            return response.body();
        }
        catch(const std::exception& e)
        {
            _logger.log_file(LogLevel::ERROR, "Query Ollama Qwen3 Error: " + std::string(e.what()));
            return "";
        }
        
    }


    std::string AIModel::query_ai(const std::string & prompt){
        std::string ck = combine_key();
        if(ck == "1_1"){
            return query_ollama_qwen(prompt);
        }else{
            _logger.log_file(LogLevel::WARNING, "No fit Platform::Model.");
            return "";
        }
    }


    json AIModel::str_to_json(const std::string & answer){

        //handle the answer
        _logger.log_file(LogLevel::INFO, "Return By AI : \n" + answer);
        //do somethings
        json answer_json = json::parse(answer);
        std::string ai_response = answer_json.value("response", "");
        if(ai_response.empty()){
            _logger.log_file(LogLevel::ERROR, "Parsed Faile Return By AI.");
        }else{
            _logger.log_file(LogLevel::INFO, "Paresed AI result: \n" + ai_response);
        }


        auto pos = ai_response.find('[');
        if(pos == std::string::npos){
            _logger.log_file(LogLevel::ERROR, "AI Return is Wrong.");
            // return default value;
        }else{
            ai_response = ai_response.substr(pos);
            _logger.log_file(LogLevel::INFO, "Substr AI result: \n" + ai_response);

        }

        json res_arr_json = json::array();
        try
        {
            /* code */
            answer_json = json::parse(ai_response);
            for(const auto & item : answer_json){
                res_arr_json.push_back(item);
            }

        }
        catch(const std::exception& e)
        {
            _logger.log_file(LogLevel::ERROR, "Construced JSON::Array II Error: " + std::string(e.what()));
        }
        

        return res_arr_json;

        
        
        // return json::array({
        //     {{"name","故宫"},{"cost",100},{"time","3小时"}},
        //     {{"name","天安门广场"},{"cost",0},{"time","1小时"}},
        //     {{"name","颐和园"},{"cost",50},{"time","2小时"}}
        // });
    }

    

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
            _location = parsed_json.value("location", "");
            _travel_days = parsed_json.value("days", 0);
            _travel_budget = parsed_json.value("budget",0.0);
        }
        catch(const std::exception& e)
        {
            _logger.log_file(LogLevel::ERROR, "Parsed Failed: " + std::string(e.what()));
            return;
        }
        
    }


    JsonErrorType Recommander::check_data(){
        if(_location.empty() || 0 == _travel_days || 0.0 == _travel_budget){
            _logger.log_file(LogLevel::WARNING, "Parsed Failed.");
            return JsonErrorType::INVALID;
        }


        if(_location.size() > 50 || 0 > _travel_days || 0.0 > _travel_budget){
            _logger.log_file(LogLevel::WARNING, "Data is not fit.");
            return JsonErrorType::NOTFIT;
        }

        //....


        return JsonErrorType::SUCCESS;
    }


    std::string Recommander::get_prompt(){
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << _travel_budget;
        std::string budget = ss.str();
        std::string prompt = 
            "你是一位旅游领域的专家，请根据我的需求提供一套旅行方案。\n"
            "目的城市：" + _location + "\n"
            "旅行天数：" + std::to_string(_travel_days) + "天\n"
            "预算：" + budget + "元\n\n"
            "请严格按照一下JSON格式返回，不要有任何额外的文字、说明或标记：\n" 
            "[{\"nth_day\":1, \"timespan\":\"上午\",\"attraction\":\"景点\",\"cost\": 400}]\n"
            "要求：\n"
            "1. 只返回JSON数组，不要有其他内容\n"
            "2. 所有字符串都用双引号\n"
            "3. 不要有尾随逗号\n"
            "4. 金额使用数字, 不要带货币符号\n"
            "5. 总花费不要超过预算";

        return prompt;
    }


    json Recommander::recommand_algorithm(){
        std::string prompt = get_prompt();
        _logger.log_file(LogLevel::INFO, "Prompt is : \n" + prompt);
        auto p_ai_model = std::make_shared<AIModel>(PlatformType::OLLama, ModelType::QWEN, _logger);
        return p_ai_model->str_to_json(p_ai_model->query_ai(prompt));
    }


    //Recommand Tools

}