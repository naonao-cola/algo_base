#include "ThreadPool.h"
#include "CrashCatch.h"
#include "logger.h"
#include <exception>


ThreadPool::ThreadPool(size_t threads)
: stop(false)
{
    for(size_t i = 0;i<threads;++i) {
        workers.emplace_back(
            [this]
            {
                CrashCatch::InitializeSymbolHandler();
                CrashCatch::LoadDllSymbols();
                SetUnhandledExceptionFilter(CrashCatch::MyUnhandledExceptionFilter);

                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        
                        // if(this->stop && this->tasks.empty())
                        //     return;
                        if(this->stop)
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
    }
}

void ThreadPool::exit()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

// the destructor joins all threads
ThreadPool::~ThreadPool()
{
    if (!stop)
    {
        exit();
    }
}
