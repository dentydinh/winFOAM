#include "core/ThreadPool.hpp"

namespace winfoam::core {

ThreadPool::ThreadPool(size_t threads) {
    if (threads == 0) threads = 1;
    workers_.reserve(threads);
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) return;
                    task = std::move(tasks_.front());
                    tasks_.pop();
                    active_tasks_.fetch_add(1, std::memory_order_relaxed);
                }
                task();
                active_tasks_.fetch_sub(1, std::memory_order_relaxed);
                idle_cv_.notify_one();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (auto& w : workers_) {
        if (w.joinable()) w.join();
    }
}

size_t ThreadPool::size() const noexcept {
    return workers_.size();
}

size_t ThreadPool::queue_size() const noexcept {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::wait_idle() noexcept {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    idle_cv_.wait(lock, [this] { return tasks_.empty() && active_tasks_.load() == 0; });
}

} // namespace winfoam::core