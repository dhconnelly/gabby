#include "utils/thread_pool.h"

#include <format>

#include "utils/logging.h"

namespace gabby {

ThreadPool::ThreadPool(int num) : threads_(num) {
    if (num < 1) {
        throw std::invalid_argument("minimum thread pool size is 1");
    }
    LOG(DEBUG) << std::format("starting {} threads", num);
    for (int i = 0; i < num; i++) {
        threads_[i] = std::thread(&ThreadPool::ThreadRun, this, i);
    }
}

void ThreadPool::ThreadRun(int id) {
    LOG(DEBUG) << std::format("thread {} starting", id);
    while (true) {
        std::unique_lock guard(mux_);
        cnd_.wait(guard, [this] { return done_ || !tasks_.empty(); });
        if (done_) return;
        LOG(DEBUG) << std::format("thread {} picking up task", id);
        auto task = tasks_.front();
        tasks_.pop_front();
        task();
    }
    LOG(DEBUG) << std::format("thread {} stopping", id);
}

ThreadPool::~ThreadPool() {
    LOG(DEBUG) << "shutting down all threads";
    {
        std::lock_guard guard(mux_);
        done_ = true;
    }
    cnd_.notify_all();
    for (auto& thread : threads_) {
        thread.join();
    }
    LOG(DEBUG) << "all threads shut down";
}

void ThreadPool::Offer(std::function<void()> task) {
    LOG(DEBUG) << "adding task to queue";
    {
        std::lock_guard guard(mux_);
        tasks_.push_back(task);
    }
    cnd_.notify_one();
    LOG(DEBUG) << "added task to queue";
}

}  // namespace gabby
