#include <thread>
#include <numeric>
#include <iostream>
#include <vector>
#include <algorithm>
#include <atomic>

template <typename Iterator, typename T>
void accumulate_block(Iterator begin, Iterator end, T init, std::atomic<T>& result) {
    result += std::accumulate(begin, end, init);
}

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    auto length = std::distance(first, last);
    auto num_workers = 8;
    if (length < 32) {
        return std::accumulate(first, last, init);
    }

    auto length_per_thread = length / num_workers;

    std::vector<std::thread> threads;
    threads.reserve(num_workers - 1);
    std::atomic<T> result;
    result = init;
    for(auto i = 0u; i < num_workers - 1; i++) {
        threads.push_back(std::thread(accumulate_block<Iterator, T>,
                std::next(first, i * length_per_thread),
                std::next(first, (i + 1) * length_per_thread),
                0, // init
                std::ref(result))); // для записи результата
    }
    result += std::accumulate(std::next(first, (num_workers - 1) * length_per_thread), last, 0);

    std::for_each(std::begin(threads),std::end(threads),std::mem_fun_ref(&std::thread::join));
    return result;
}

int main() {
    std::vector<int> test_sequence(100);
    std::iota(std::begin(test_sequence), std::end(test_sequence), 0);
    std::cout << parallel_accumulate(std::begin(test_sequence),std::end(test_sequence),
                                     0)<< std::endl << std::endl;

}