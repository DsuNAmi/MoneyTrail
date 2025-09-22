#include "includes/threadpool.hpp"


// ThreadPool
ThreadPool::ThreadPool(int thread_number, Logger & logger,int max_thread_number, int thread_add_chunk)
: _max_thread_number(max_thread_number), _thread_add_chunk(thread_add_chunk),
  _thread_number(init_thread_number(thread_number)),
  _stop(false),
  _logger(logger)
{
    _logger.log_file(LogLevel::INFO, "ThreadPool initiallized with " + std::to_string(_thread_number) + " threads.");
}


ThreadPool::~ThreadPool(){
    stop();
    _logger.log_file(LogLevel::INFO, "ThreadPool stopped. All things cleaned up.");
}


void ThreadPool::running(int t_number){
    for(int i = 0; i < t_number; ++i){
        _threadpool.emplace_back([this](){
            while(!is_stop()){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _condition.wait(lock, [this]{
                        return is_stop() || !_tasks.empty();
                    });

                    if(is_stop() && _tasks.empty()){
                        return;
                    }

                    task = std::move(_tasks.front());
                    _tasks.pop();
                }
                task();
            }
        });
    }
}

void ThreadPool::add_chunk(){
    if(is_stop()){
        _logger.log_file(LogLevel::WARNING, "ThreadPool is stopped. Cannot add more threads.");
        return;
    }
    if(_thread_number >= _max_thread_number){
        _logger.log_file(LogLevel::WARNING, "ThreadPool has reached max thread number. Cannot add more threads.");
        return;
    }
    int new_thread = _thread_number + _thread_add_chunk;
    if(new_thread > _max_thread_number){
        new_thread = _max_thread_number;
    }
    int real_add = new_thread - _thread_number;
    running(real_add);
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _thread_number = new_thread;
    }
    _logger.log_file(LogLevel::INFO, "ThreadPool added " + std::to_string(real_add) + " threads. Current thread number: " + std::to_string(_thread_number));
}

// void ThreadPool::remove_chunk(){
//     if(is_stop()){
//         _logger.log_file(LogLevel::WARNING, "ThreadPool is stopped. Cannot remove threads.");
//         return;
//     }
//     if(_thread_number <= _thread_add_chunk){
//         _logger.log_file(LogLevel::WARNING, "ThreadPool has reached min thread number. Cannot remove more threads.");
//         return;
//     }
//     int new_thread = _thread_number - _thread_add_chunk;
//     if(0 == new_thread){
//         new_thread = 1;
//     }
//     int real_remove = _thread_number - new_thread;
//     {
//         std::lock_guard<std::mutex> lock(_mutex);
//         _thread_number = new_thread;
//     }
//     for(int i = 0; i < real_remove; ++i){
//         add_task([this]{
//             _stop.store(true);
//             _condition.notify_one();
//         });
//     }

// }   


void ThreadPool::stop(){
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop.store(true);
    }

    _condition.notify_all();
    for(auto & th : _threadpool){
        if(th.joinable()){
            th.join();
        }
    }
}
