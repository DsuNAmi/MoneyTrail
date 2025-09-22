#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <queue>


#include "logger.hpp"

class ThreadPool {
    public:
        ThreadPool(int thread_number, Logger & logger, int max_thread_number, int thread_add_chunk);
        virtual ~ThreadPool();

        ThreadPool(const ThreadPool &) = delete;
        ThreadPool & operator=(const ThreadPool &) = delete;

        void start() {running(_thread_number);}
        void extend() {add_chunk();}
        void enqueue(std::function<void()> task){
            add_task(std::move(task));
        }


    private:
        template<typename Func , typename... Args>
        void add_task(Func && function, Args && ... args){
            auto task_wrapper = [
                function_copy = std::forward<Func>(function),
                ... args_copy = std::forward<Args>(args)
            ]() mutable {
                function_copy(args_copy...);
            };

            {
                std::unique_lock<std::mutex> lock(_mutex);
                _tasks.emplace(std::move(task_wrapper));
            }
            _condition.notify_one();
        }

    public:
        void add_chunk();
        // void remove_chunk();
        void stop();

    public:
        bool is_stop() const {return _stop.load();}
        int size() const {return _thread_number;}

    private:
        int init_thread_number(int thread_number){
            if(thread_number > 200 || thread_number <= 0){
                return _thread_add_chunk;
            }
            return thread_number;
        }
        void running(int t_number);

    private:
        int _max_thread_number;
        int _thread_add_chunk;
        int _thread_number;

        std::vector<std::thread> _threadpool;

        std::queue<std::function<void()>> _tasks;
        std::mutex _mutex;
        std::condition_variable _condition;

        std::atomic<bool> _stop;

        Logger & _logger;


};