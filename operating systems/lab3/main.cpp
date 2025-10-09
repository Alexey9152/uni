#include "marker_thread.h"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

#ifndef TESTING

int main() {
    int array_size;
    std::cout << "Enter array size: ";
    std::cin >> array_size;
    
    std::vector<int> array(array_size, 0);
    
    int thread_count;
    std::cout << "Enter number of marker threads: ";
    std::cin >> thread_count;
    
    std::mutex array_mutex;
    std::condition_variable cv;
    std::condition_variable main_cv;
    
    std::vector<std::unique_ptr<MarkerThread>> threads;
    std::vector<bool> thread_waiting(thread_count, false);
    std::vector<bool> thread_finished(thread_count, false);
    
    for (int i = 0; i < thread_count; ++i) {
        threads.push_back(std::make_unique<MarkerThread>(i + 1, array, array_mutex, cv, main_cv));
    }
    
    for (auto& thread : threads) {
        thread->start();
    }
    
    std::cout << "All threads started" << std::endl;
    
    int active_threads = thread_count;
    
    while (active_threads > 0) {
        {
            std::unique_lock<std::mutex> lock(array_mutex);
            main_cv.wait(lock, [&]() {
                int waiting_count = 0;
                for (int i = 0; i < thread_count; ++i) {
                    if (threads[i]->isWaiting() && !thread_finished[i]) {
                        waiting_count++;
                    }
                }
                return waiting_count == active_threads;
            });
        }
        
        int thread_to_stop;
        std::cout << "Enter thread number to stop: ";
        std::cin >> thread_to_stop;
        
        if (thread_to_stop >= 1 && thread_to_stop <= thread_count && 
            !thread_finished[thread_to_stop - 1]) {
            
            threads[thread_to_stop - 1]->stop();
            thread_finished[thread_to_stop - 1] = true;
            active_threads--;
            
            std::cout << "\nArray contents after stopping thread " << thread_to_stop << ":" << std::endl;

            
            for (int i = 0; i < thread_count; ++i) {
                if (!thread_finished[i] && threads[i]->isWaiting()) {
                    threads[i]->resume();
                }
            }

        }
    }
    
    std::cout << "All threads finished. Program completed." << std::endl;
    
    return 0;
}

#endif