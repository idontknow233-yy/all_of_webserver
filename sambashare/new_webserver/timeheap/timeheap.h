#ifndef TIME_HEAP_H
#define TIME_HEAP_H

#include <functional>
#include <vector>
#include <chrono>
#include <time.h>
#include <unordered_map>
#include <assert.h>
#include <algorithm>

typedef std::function<void()> TimeoutCallback;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode
{
    int id;
    TimeoutCallback call_back;
    TimeStamp node;
    bool operator<(const TimerNode &temp)
    {
        return node < temp.node;
    }
    bool operator>(const TimerNode &temp)
    {
        return node > temp.node;
    }
};

class timeheap {
public:
    timeheap() { heap.reserve(64); };
    ~timeheap() { Clear(); };
    void Clear();

    void insert(int idx, int timeout, TimeoutCallback &callbakc_);
    void del(int pos);
    void adjust(int idx, int timeout);
    void DoWork(int idx);
    void tick();
    void pop();
    int GetNextTick();

private:
    void turn_down(int pos);
    void turn_up(int pos);
    void SwapNode(int pos1, int pos2);


    std::vector<TimerNode> heap;
    std::unordered_map<int, int> reflect;
};

#endif