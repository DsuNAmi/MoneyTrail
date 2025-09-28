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
            _port = 11431;
        }
    }

    std::string AIModel::query_ollama_qwen(const std::string & prompt){
        try
        {
            auto const result = _resolver.resolve(_host, std::to_string(_port));
            _stream.connect(result);
    
    
            std::string body = R"({"model":"qwen3:8b","prompt":")" + prompt + R"("})";
    
    
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
        _logger.log_file(LogLevel::INFO, "Return By AI : \n" + answer);
        //do somethings
        return json::array({
            {{"name","故宫"},{"cost",100},{"time","3小时"}},
            {{"name","天安门广场"},{"cost",0},{"time","1小时"}},
            {{"name","颐和园"},{"cost",50},{"time","2小时"}}
        });
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
        std::string prompt = json_to_str(res_json);
        auto p_ai_model = std::make_shared<AIModel>(PlatformType::OLLama, ModelType::QWEN, _logger);
        return p_ai_model->str_to_json(p_ai_model->query_ai(prompt));
    }

    std::string Recommander::json_to_str(const json & res_json){
        std::string changed = res_json.dump();
        _logger.log_file(LogLevel::INFO, "Change to str: \n" + changed);
        return "请给我推荐一个天津2日游的行程，预算1800元";
    }


    //Recommand Tools

}