#include "marker_thread.h"
#include <iostream>
#include <cstdlib>
#include <chrono>

MarkerThread::MarkerThread(int thread_id,
                           std::vector<int>& arr,
                           boost::mutex& mutex,
                           boost::condition_variable& cont_cv,
                           boost::condition_variable& main_cv)
    : id(thread_id),
      array(arr),
      array_mutex(mutex),
      continue_cv(cont_cv),
      main_ready_cv(main_cv) {
    std::srand(id); // по условию лабы
}

MarkerThread::~MarkerThread() {
    stop();
}

void MarkerThread::run() {
    while (running.load()) {
        // захватываем mutex единообразно
        boost::unique_lock<boost::mutex> lock(array_mutex);

        if (array.empty()) {
            break;
        }

        int index = std::rand() % array.size();

        if (array[index] == 0) {
            // элемент свободен — помечаем, как в ТЗ
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            lock.lock();

            if (!running.load()) break;

            array[index] = id;
            marked_count++;

            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            // на следующей итерации цикл опять захватит lock
        } else {
            // элемент занят — сообщаем main и ждём его решения
            std::cout << "Thread " << id
                      << ": marked " << marked_count
                      << " elements, cannot mark index " << index
                      << std::endl;

            waiting_for_signal.store(true);
            main_ready_cv.notify_one(); // сообщаем main, что ждём

            continue_cv.wait(lock, [this] {
                return !waiting_for_signal.load() || !running.load();
            });

            if (!running.load()) {
                break;
            }

            // main сказал продолжать
            waiting_for_signal.store(false);
        }
        // lock выходит из области *в конце while* и отпускает mutex
    }

    // очистить свои метки
    {
        boost::unique_lock<boost::mutex> lock(array_mutex);
        for (size_t i = 0; i < array.size(); ++i) {
            if (array[i] == id) {
                array[i] = 0;
            }
        }
        finished.store(true);
    }
}

void MarkerThread::start() {
    if (running.load()) return;
    running.store(true);
    thread_obj = std::thread(&MarkerThread::run, this);
}

void MarkerThread::stop() {
    if (!running.load()) {
        if (thread_obj.joinable())
            thread_obj.join();
        return;
    }

    {
        boost::unique_lock<boost::mutex> lock(array_mutex);
        running.store(false);
        waiting_for_signal.store(false);
        continue_cv.notify_all();
    }

    if (thread_obj.joinable()) {
        thread_obj.join();
    }
}