#include <thread>
#include <numeric>
#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>

std::mutex result_mutex;

template <typename Iterator, typename T>
void accumulate_block(Iterator first, Iterator last, T& init) {
    std::lock_guard<std::mutex> my_lock_guard(result_mutex);
    std::this_thread::sleep_for(std::chrono::seconds (1));
    init = std::accumulate(first, last, init);
}

template<typename Iterator, typename T>
void parallel_accumulate(Iterator first, Iterator last, T& init) {
    auto length = std::distance(first, last);
    if (length < 32) {
        init = std::accumulate(first, last, init);
        return;
    }
    auto num_workers = std::thread::hardware_concurrency();
    auto length_per_thread = (length + num_workers - 1) / num_workers;
    std::vector<std::thread> threads;
    std::vector<T> results(num_workers - 1);
    for(auto i = 0u; i < num_workers - 1; i++) {
        auto beginning = std::next(first, i * length_per_thread);
        auto ending = std::next(first, (i + 1) * length_per_thread);
        threads.push_back(std::thread(
                accumulate_block<Iterator, T>,
                beginning, ending, std::ref(init)));
    }
    init = std::accumulate(std::next(first, (num_workers - 1) * length_per_thread),
                                       last, init);
    std::for_each(std::begin(threads), std::end(threads), std::mem_fun_ref(&std::thread::join));
}

int main() {
    std::vector<int> test_sequence(100u);
    std::iota(test_sequence.begin(), test_sequence.end(), 0);
    auto result = 0;
    parallel_accumulate(std::begin(test_sequence), std::end(test_sequence), result);
    std::cout << "Result is " << result << std::endl;
}