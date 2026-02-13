#ifndef MQ_THREAD_POOL_H
#define MQ_THREAD_POOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <atomic>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::condition_variable doneCondition;
    bool stop = false;
    std::atomic<int> activeTasks{0};

public:
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty())
                            return;

                        if (!tasks.empty()) {
                            task = std::move(tasks.front());
                            tasks.pop();
                            activeTasks++;
                        }
                    }

                    if (task) {
                        task();
                        activeTasks--;
                        doneCondition.notify_all();
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    size_t pendingTasks() {
        std::unique_lock<std::mutex> lock(queueMutex);
        return tasks.size();
    }

    void waitAll() {
        std::unique_lock<std::mutex> lock(queueMutex);
        doneCondition.wait(lock, [this] {
            return tasks.empty() && activeTasks.load() == 0;
        });
    }
};

#endif // MQ_THREAD_POOL_H
