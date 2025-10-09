#ifndef MARKER_THREAD_H
#define MARKER_THREAD_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>

class MarkerThread {
public:
    MarkerThread(int id, std::vector<int>& array, 
                 std::mutex& array_mutex, 
                 std::condition_variable& cv,
                 std::condition_variable& main_cv);
    
    void start();
    void stop();
    void resume();
    void waitForSignal();
    bool isWaiting() const;
    bool isFinished() const;
    int getMarkedCount() const;
    int getId() const;

private:
    void run();

    int id_;
    std::vector<int>& array_;
    std::mutex& array_mutex_;
    std::condition_variable& cv_;
    std::condition_variable& main_cv_;
    
    std::thread thread_;
    std::mutex thread_mutex_;
    std::condition_variable thread_cv_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> finished_{false};
    std::atomic<bool> waiting_{false};
    std::atomic<bool> should_stop_{false};
    std::atomic<int> marked_count_{0};
    
    std::random_device rd_;
    std::mt19937 gen_;
};

#endif // MARKER_THREAD_H