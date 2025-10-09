#include "marker_thread.h"
#include <iostream>
#include <chrono>

MarkerThread::MarkerThread(int id, std::vector<int>& array, 
                         std::mutex& array_mutex, 
                         std::condition_variable& cv,
                         std::condition_variable& main_cv)
    : id_(id), array_(array), array_mutex_(array_mutex), 
      cv_(cv), main_cv_(main_cv), gen_(rd_()) {
}

void MarkerThread::start() {
    running_ = true;
    finished_ = false;
    waiting_ = false;
    should_stop_ = false;
    marked_count_ = 0;
    
    thread_ = std::thread(&MarkerThread::run, this);
}

void MarkerThread::stop() {
    {
        std::unique_lock<std::mutex> lock(thread_mutex_);
        should_stop_ = true;
        thread_cv_.notify_one();
    }
    
    if (thread_.joinable()) {
        thread_.join();
    }
}

void MarkerThread::resume() {
    std::unique_lock<std::mutex> lock(thread_mutex_);
    waiting_ = false;
    thread_cv_.notify_one();
}

void MarkerThread::waitForSignal() {
    std::unique_lock<std::mutex> lock(thread_mutex_);
    thread_cv_.wait(lock, [this]() { return !waiting_ || should_stop_; });
}

bool MarkerThread::isWaiting() const {
    return waiting_;
}

bool MarkerThread::isFinished() const {
    return finished_;
}

int MarkerThread::getMarkedCount() const {
    return marked_count_;
}

int MarkerThread::getId() const {
    return id_;
}

void MarkerThread::run() {
    std::uniform_int_distribution<> dis(0, array_.size() - 1);
    
    while (running_ && !should_stop_) {
        int index = dis(gen_);
        
        {
            std::unique_lock<std::mutex> lock(array_mutex_);
            
            if (array_[index] == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                array_[index] = id_;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                marked_count_++;
            } else {
                std::cout << "Thread " << id_ 
                          << ": marked " << marked_count_ 
                          << " elements, cannot mark index " << index << std::endl;
                
                waiting_ = true;
                main_cv_.notify_one();
                
                lock.unlock();
                waitForSignal();
                lock.lock();
                
                if (should_stop_) {
                    break;
                }
            }
        }
        
        if (should_stop_) {
            break;
        }
    }
    
    if (should_stop_) {
        std::unique_lock<std::mutex> lock(array_mutex_);
        for (size_t i = 0; i < array_.size(); ++i) {
            if (array_[i] == id_) {
                array_[i] = 0;
            }
        }
    }
    
    finished_ = true;
    running_ = false;
}