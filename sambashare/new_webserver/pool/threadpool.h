#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <thread>
#include <assert.h>

class threadpool
{
public:
    threadpool(int threadcount = 8)
    {
        pool_ptr = std::make_shared<pool>();
        pool_ptr->isClose = false;
        for (int i = 0; i < threadcount; i++)
        {
            std::thread([this]()
                        {
                std::unique_lock<std::mutex> locker(pool_ptr->mtx);
                while(true)
                {
                    if(!pool_ptr->task.empty())
                    {
                        auto temp_task = std::move(pool_ptr->task.front());
                        pool_ptr->task.pop();
                        locker.unlock();
                        temp_task();
                        locker.lock();
                    }
                    else if(pool_ptr->isClose)
                    {
                        break;
                    }
                    else
                    {
                        pool_ptr->cond.wait(locker);
                    }
                } })
                .detach();
        }
    }

    ~threadpool()
    {
        std::lock_guard<std::mutex> locker(pool_ptr->mtx);
        pool_ptr->isClose = true;
        pool_ptr->cond.notify_all();
    }

    template <typename T>
    void add_task(T &&task)
    {
        std::lock_guard<std::mutex> locker(pool_ptr->mtx);
        pool_ptr->task.emplace(std::forward<T>(task));
        pool_ptr->cond.notify_one();
    }

private:
    struct pool
    {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClose;
        std::queue<std::function<void()>> task;
    };
    std::shared_ptr<pool> pool_ptr;
};