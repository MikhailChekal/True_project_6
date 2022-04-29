#include <thread>
#include <numeric>
#include <iostream>
#include <vector>
#include <algorithm>
#include "timer.h"
#include <mutex>

std::mutex result_mt;

template <typename Iterator, typename T>
void accumulate_block(Iterator first, Iterator last, T init, T& result) {
    std::lock_guard<std::mutex> lockGuard(result_mt);
    result = std::accumulate(first, last, init);
}

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    auto length = std::distance(first, last);
    if (length < 32) {
        std::cout << "no multithread now" << std::endl;
        return std::accumulate(first, last, init);
    }
    auto num_workers = std::thread::hardware_concurrency();
    auto length_per_thread = (length) / num_workers;

    std::vector<std::thread> threads;
    std::vector<T> results(num_workers - 1);

    for(auto i = 0u; i < num_workers - 1; i++) {
        auto beginning = std::next(first, i * length_per_thread);
        auto ending = std::next(first, (i + 1) * length_per_thread);
        threads.push_back(std::thread(
                accumulate_block<Iterator, T>,
                beginning, ending, 0, std::ref(results[i])));
    }
    auto main_result = std::accumulate(std::next(first, (num_workers - 1) * length_per_thread),
                                       last, init);
    std::for_each(std::begin(threads), std::end(threads), std::mem_fun_ref(&std::thread::join));
    return std::accumulate(std::begin(results), std::end(results), main_result);
}

int main() {
    std::vector<int> test_sequence(100u);
    std::iota(test_sequence.begin(), test_sequence.end(), 0);
    auto result = 0;
    std::cout << "Result is" << parallel_accumulate(std::begin(test_sequence), std::end(test_sequence), result);
    }