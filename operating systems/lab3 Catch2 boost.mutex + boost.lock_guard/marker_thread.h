#ifndef MARKER_THREAD_H
#define MARKER_THREAD_H

#include <vector>
#include <thread>
#include <atomic>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

class MarkerThread {
private:
    int id;
    std::vector<int>& array;
    boost::mutex& array_mutex;
    boost::condition_variable& continue_cv;   // main -> marker
    boost::condition_variable& main_ready_cv; // marker -> main

    std::thread thread_obj;
    std::atomic<bool> running{false};
    std::atomic<bool> finished{false};
    std::atomic<bool> waiting_for_signal{false};
    int marked_count{0};

    void run();

public:
    MarkerThread(int thread_id,
                 std::vector<int>& arr,
                 boost::mutex& mutex,
                 boost::condition_variable& cont_cv,
                 boost::condition_variable& main_cv);
    ~MarkerThread();

    void start();
    void stop();

    bool isWaitingForSignal() const { return waiting_for_signal.load(); }
    bool isFinished() const { return finished.load(); }
    int getId() const { return id; }
    int getMarkedCount() const { return marked_count; }
};

#endif // MARKER_THREAD_H
