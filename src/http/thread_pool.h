#ifndef GABBY_HTTP_THREAD_POOL_H_
#define GABBY_HTTP_THREAD_POOL_H_

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace gabby {

using Task = std::function<void()>;

// a producer-consumer thread pool
class ThreadPool {
public:
    ThreadPool(int num);
    ~ThreadPool();
    void Offer(Task task);

private:
    void ThreadRun(int id);

    std::mutex mux_;
    bool done_ = false;
    std::deque<Task> tasks_;
    std::condition_variable cnd_;
    std::vector<std::thread> threads_;
};

}  // namespace gabby

#endif  // GABBY_HTTP_THREAD_POOL_H_
