#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <sys/time.h>
#include <string>
#include <cstring>

template <typename T>
class blockqueue
{
public:
    blockqueue(size_t capacity_size = 500);
    ~blockqueue();

    void push_back(const T &str);
    void push_front(const T &str);

    bool pop(T &str);
    bool pop(T &str, int timeout);

    bool empty();
    bool full();

    T front();
    T back();
    size_t capacity();
    size_t size();

    void Close();
    void flush();

    bool isClose_;

    std::mutex mtx;

private:
    std::deque<T> deq_;
    size_t capacity_;
    std::condition_variable condConsumer;
    std::condition_variable condProducer;
};

template <typename T>
blockqueue<T>::blockqueue(size_t capacity_size) : capacity_(capacity_size)
{
    isClose_ = false;
}

template <typename T>
blockqueue<T>::~blockqueue()
{
    std::lock_guard<std::mutex> locker(mtx);
    deq_.clear();
    isClose_ = true;
    condConsumer.notify_all();
    condProducer.notify_all();
}

template <typename T>
void blockqueue<T>::push_back(const T &str)
{
    std::unique_lock<std::mutex> locker(mtx);
    while (deq_.size() >= capacity_)
    {
        condConsumer.wait(locker); // 生产者等待消费者腾出空间
    }
    deq_.push_back(str);
    condProducer.notify_one(); // 通知消费者有新元素
}

template <typename T>
void blockqueue<T>::push_front(const T &str)
{
    std::unique_lock<std::mutex> locker(mtx);
    while (deq_.size() >= capacity_)
    {
        condConsumer.wait(locker);
    }
    deq_.push_front(str);
    condProducer.notify_one();
}

template <typename T>
bool blockqueue<T>::pop(T &str)
{
    std::unique_lock<std::mutex> locker(mtx);
    while (deq_.empty())
    {
        condProducer.wait(locker); // 消费者等待生产者添加元素
        if (isClose_)
            return false;
    }
    str = deq_.front();
    condConsumer.notify_one(); // 通知生产者有可用空间
    deq_.pop_front();
    return true;
}

template <typename T>
bool blockqueue<T>::pop(T &str, int timeout)
{
    std::unique_lock<std::mutex> locker(mtx);
    while (deq_.empty())
    {
        if (condProducer.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)
        {
            return false;
        }
        if (isClose_)
            return false;
    }
    str = deq_.front();
    condConsumer.notify_one();
    deq_.pop_front();
    return true;
}

template <typename T>
bool blockqueue<T>::empty()
{
    std::lock_guard<std::mutex> locker(mtx);
    return deq_.empty();
}

template <typename T>
bool blockqueue<T>::full()
{
    std::lock_guard<std::mutex> locker(mtx);
    return deq_.size() >= capacity_;
}

template <typename T>
T blockqueue<T>::front()
{
    std::lock_guard<std::mutex> locker(mtx);
    return deq_.front();
}

template <typename T>
T blockqueue<T>::back()
{
    std::lock_guard<std::mutex> locker(mtx);
    return deq_.back();
}

template <typename T>
size_t blockqueue<T>::capacity()
{
    std::lock_guard<std::mutex> locker(mtx);
    return capacity_;
}

template <typename T>
size_t blockqueue<T>::size()
{
    std::lock_guard<std::mutex> locker(mtx);
    return deq_.size();
}

template <typename T>
void blockqueue<T>::Close()
{
    std::lock_guard<std::mutex> locker(mtx); // 操控队列之前，都需要上锁
    deq_.clear();                            // 清空队列
    isClose_ = true;
    condConsumer.notify_all();
    condProducer.notify_all();
}

template <typename T>
void blockqueue<T>::flush()
{
    std::lock_guard<std::mutex> locker(mtx);
    condConsumer.notify_one();
}

#endif