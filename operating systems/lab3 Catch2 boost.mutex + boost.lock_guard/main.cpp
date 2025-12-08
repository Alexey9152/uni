#include "marker_thread.h"
#include <iostream>
#include <vector>
#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

int main() {
    int array_size;
    std::cout << "Enter array size: ";
    std::cin >> array_size;
    std::vector<int> array(array_size, 0);

    int thread_count;
    std::cout << "Enter number of marker threads: ";
    std::cin >> thread_count;

    boost::mutex array_mutex;
    boost::condition_variable main_ready_cv;  // marker -> main
    boost::condition_variable continue_cv;    // main -> marker

    std::vector<std::unique_ptr<MarkerThread>> threads;
    std::vector<bool> thread_finished(thread_count, false);

    for (int i = 0; i < thread_count; ++i) {
        threads.push_back(std::make_unique<MarkerThread>(
            i + 1, array, array_mutex, continue_cv, main_ready_cv));
    }

    for (auto& t : threads) {
        t->start();
    }

    std::cout << "All threads started" << std::endl;

    int active_threads = thread_count;
    while (active_threads > 0) {
        // ждём, пока ВСЕ активные потоки не уйдут в ожидание
        {
            boost::unique_lock<boost::mutex> lock(array_mutex);
            main_ready_cv.wait(lock, [&] {
                int waiting_count = 0;
                for (int i = 0; i < thread_count; ++i) {
                    if (!thread_finished[i] && threads[i]->isWaitingForSignal()) {
                        waiting_count++;
                    }
                }
                return waiting_count == active_threads;
            });
        }

        // печать массива
        std::cout << "\nArray contents:" << std::endl;
        {
            boost::unique_lock<boost::mutex> lock(array_mutex);
            for (int v : array) std::cout << v << ' ';
            std::cout << std::endl;
        }

        int thread_to_stop;
        std::cout << "Enter thread number to stop (1-" << thread_count << "): ";
        std::cin >> thread_to_stop;

        if (thread_to_stop >= 1 && thread_to_stop <= thread_count &&
            !thread_finished[thread_to_stop - 1]) {

            threads[thread_to_stop - 1]->stop();
            thread_finished[thread_to_stop - 1] = true;
            active_threads--;

            std::cout << "Array after stopping thread " << thread_to_stop << ":\n";
            {
                boost::unique_lock<boost::mutex> lock(array_mutex);
                for (int v : array) std::cout << v << ' ';
                std::cout << std::endl;
            }

            // сказать остальным «продолжаем»
            {
                boost::unique_lock<boost::mutex> lock(array_mutex);
                continue_cv.notify_all();
            }
        } else {
            std::cout << "Invalid thread number or already finished.\n";
            {
                boost::unique_lock<boost::mutex> lock(array_mutex);
                continue_cv.notify_all();
            }
        }
    }

    std::cout << "All threads finished. Program completed.\n";
    return 0;
}
