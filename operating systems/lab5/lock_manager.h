// lock_manager.h
#pragma once
#include <map>
#include <mutex>

class LockManager {
public:
    // Разрешить чтение: true, если нет писателя.
    bool acquireRead(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto &st = table_[id];
        if (st.writer) return false;
        ++st.readers;
        return true;
    }

    // Разрешить запись: true, если нет ни читателей, ни писателя.
    bool acquireWrite(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto &st = table_[id];
        if (st.writer || st.readers > 0) return false;
        st.writer = true;
        return true;
    }

    void releaseRead(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = table_.find(id);
        if (it == table_.end()) return;
        if (it->second.readers > 0) --it->second.readers;
        cleanupIfEmpty(id, it->second);
    }

    void releaseWrite(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = table_.find(id);
        if (it == table_.end()) return;
        it->second.writer = false;
        cleanupIfEmpty(id, it->second);
    }

private:
    struct State {
        int  readers = 0;
        bool writer  = false;
    };

    std::map<int, State> table_;
    std::mutex           mutex_;

    void cleanupIfEmpty(int id, const State &st) {
        if (!st.writer && st.readers == 0) {
            table_.erase(id);
        }
    }
};
