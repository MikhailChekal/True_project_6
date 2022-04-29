//
// Created by misha on 29.04.2022.
//

#include <queue>
#include <thread>
#include <mutex>
#include <iostream>




template<typename T>
class safe_queue
{
private:
    std::queue<T> queue;
    std::mutex queue_mutex;
public:
    safe_queue() = default;
    ~safe_queue() = default;

    void push(T elem)
    {
        std::lock_guard<std::mutex> lock_queue(queue_mutex);
        queue.push(elem);
    }

    template<class Arg, class ... Args>
    void emplace(const Arg& fst_arg, const Args&... rest_args)
    {
        queue.emplace(fst_arg, rest_args...);
    };

    void pop()
    {
        std::lock_guard<std::mutex> lock_queue(queue_mutex);
        queue.pop();
    }

    T& front()
    {
        std::lock_guard<std::mutex> lock_queue(queue_mutex);
        return queue.front();
    }
    T& back()
    {
        std::lock_guard<std::mutex> lock_queue(queue_mutex);
        return queue.back();
    }
    size_t size()
    {
        std::lock_guard<std::mutex> lock_queue(queue_mutex);
        return queue.size();
    }
};


safe_queue<int> theQueue;

void fill_the_Queue(int first, int last)
{
    for(int i = last; i>= first; i--) theQueue.push(i);
}

int main()
{

    for(int i = 1; i<= 10000; i++) theQueue.push(i);
    std::cout << theQueue.front() << std::endl;
    std::cout << theQueue.back() << std::endl;
    std::thread other(fill_the_Queue, 0, 10000);
    for(int i = 0; i<= 19999; i++) theQueue.pop();
    other.join();
    std::cout << theQueue.size() << std::endl;
}