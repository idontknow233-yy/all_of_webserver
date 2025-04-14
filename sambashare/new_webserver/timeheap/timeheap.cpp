#include "timeheap.h"

void timeheap::insert(int idx, int timeout, TimeoutCallback &callbakc_)
{
    if (reflect.count(idx))
    {
        int pos = reflect[idx];
        heap[pos].call_back = callbakc_;
        heap[pos].node = Clock::now() + MS(timeout);
        turn_down(idx);
        turn_up(idx);
    }
    else
    {
        heap.push_back({idx, callbakc_, Clock::now() + MS(timeout)});
        reflect[heap.size() - 1] = idx;
        turn_up(idx);
    }
}

void timeheap::del(int pos)
{
    assert(pos >= 0 && pos < heap.size());
    int id = heap[pos].id;
    if (pos != heap.size() - 1)
    {
        SwapNode(pos, heap.size() - 1);
        heap.pop_back();
        turn_down(pos);
        turn_up(pos);
    }
    else
    {
        heap.pop_back();
    }
    reflect.erase(id);
}

void timeheap::adjust(int idx, int timeout)
{
    assert(reflect.count(idx));
    int pos = reflect[idx];
    heap[pos].node = Clock::now() + MS(timeout);
    turn_down(pos);
    turn_up(pos);
}

void timeheap::DoWork(int idx)
{
    if (heap.empty() || reflect.count(idx))
        return;
    int pos = reflect[idx];
    heap[pos].call_back();
    del(pos);
}

void timeheap::tick()
{
    while (heap.size())
    {
        if (std::chrono::duration_cast<MS>(heap[0].node - Clock::now()).count() > 0)
        {
            break;
        }
        heap[0].call_back();
        pop();
    }
}

void timeheap::pop()
{
    del(0);
}

int timeheap::GetNextTick()
{
    tick();
    int ret = -1;
    if (heap.size())
    {
        ret = std::chrono::duration_cast<MS>(heap.front().node - Clock::now()).count();
        if (ret < 0)
            ret = 0;
    }
    return ret;
}

void timeheap::turn_down(int pos)
{
    while (1)
    {
        int ls = pos * 2 + 1, rs = pos * 2 + 2, owo = -1;
        if(ls >= heap.size()) break;
        if(heap[ls] < heap[pos])
        {
            owo = ls;
            if(rs < heap.size() && heap[rs] < heap[ls]) 
                owo = rs;
        }
        else 
            break;
        SwapNode(pos, owo);
        pos = owo;
    }
}

void timeheap::turn_up(int pos)
{
    while(pos)
    {
        int parent = (pos - 1) / 2;
        if(heap[pos] > heap[parent]) break;
        SwapNode(parent, pos);
        pos = parent;
    }
}

void timeheap::SwapNode(int pos1, int pos2)
{
    assert(pos1 >= 0 && pos1 < heap.size());
    assert(pos1 >= 0 && pos1 < heap.size());
    std::swap(heap[pos1], heap[pos2]);
    reflect[heap[pos1].id] = pos1;
    reflect[heap[pos2].id] = pos2;
}

void timeheap::Clear()
{
    heap.clear();
    reflect.clear();
}