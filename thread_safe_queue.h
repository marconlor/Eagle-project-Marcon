#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <string>
#include <cstdint>
#include <chrono>

using namespace std;

struct RawCANData {
    string message;
    uint64_t timestamp;
};

class ThreadSafeQueue {
private:
    queue<RawCANData> queue_;
    mutable mutex mutex_;

public:
    void push(const string& message, uint64_t timestamp) {
        lock_guard<mutex> lock(mutex_);
        queue_.push({message, timestamp});
    }

    bool tryPop(RawCANData& data) {
        lock_guard<mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        data = queue_.front();
        queue_.pop();
        return true;
    }

    bool isEmpty() const {
        lock_guard<mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        lock_guard<mutex> lock(mutex_);
        return queue_.size();
    }
};

inline uint64_t getCurrentTimestampMs() {
    using namespace chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

#endif