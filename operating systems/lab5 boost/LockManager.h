// LockManager.h
#pragma once
#include <map>
#include <boost/thread/mutex.hpp>

struct LockState {
    int  readers = 0;
    bool writer  = false;
};

class LockManager {
public:
    bool acquireRead(int id) {
        boost::lock_guard<boost::mutex> lg(mutex_);
        LockState& st = locks_[id];
        if (st.writer) return false;
        ++st.readers;
        return true;
    }

    void releaseRead(int id) {
        boost::lock_guard<boost::mutex> lg(mutex_);
        auto it = locks_.find(id);
        if (it == locks_.end()) return;
        if (it->second.readers > 0) --it->second.readers;
    }

    bool acquireWrite(int id) {
        boost::lock_guard<boost::mutex> lg(mutex_);
        LockState& st = locks_[id];
        if (st.writer || st.readers > 0) return false;
        st.writer = true;
        return true;
    }

    void releaseWrite(int id) {
        boost::lock_guard<boost::mutex> lg(mutex_);
        auto it = locks_.find(id);
        if (it == locks_.end()) return;
        it->second.writer = false;
    }

private:
    boost::mutex mutex_;
    std::map<int, LockState> locks_;
};
