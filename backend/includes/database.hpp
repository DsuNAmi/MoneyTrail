#pragma once


#include <sqlite3.h>
#include <string>


#include "logger.hpp"

class Database{
    public:
        virtual ~Database();


    private:
        std::mutex _mutex;
        // Logger & _logger;

};