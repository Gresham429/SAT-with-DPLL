#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool
{
public:
    ThreadPool(int min_threads, int max_threads) : stop(false), active_threads(0), min_threads(min_threads), max_threads(max_threads)
    {
        for (int i = 0; i < min_threads; ++i)
        {
            // 创建最小数量的线程
            CreateThread();
        }

        // // 创建管理线程
        // management_thread = std::thread([this]() {
        //     while(!stop)
        //     {
        //         std::this_thread::sleep_for(std::chrono::seconds(5));
        //         ManageThreads();
        //     }
        // });
    }

    // 析构函数
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(task_mtx);
            stop = true;
        }

        // 通知所有线程停止
        condition.notify_all();

        // 等待所有线程完成任务并且退出
        for (auto &thread : threads)
        {
            thread.join();
        }
    }

    void ManageThreads()
    {
        if (!tasks.empty() && threads.size() < max_threads)
        {
            ExpandThreads(2);
        }
        else if (active_threads < threads.size() && min_threads < threads.size())
        {
            RecycleThreads();
        }
    }

    // 添加任务到线程池
    template <typename F, typename... Args>
    auto EnqueueTask(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        // 绑定函数
        using ReturnType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<ReturnType> future = task->get_future();

        // 将任务添加到任务队列
        {
            std::unique_lock<std::mutex> lock(task_mtx);
            std::function<void()> func = [task]()
            {
                (*task)();
            };
            tasks.emplace(func);
        }

        if (HasAvailableThreads())
        {
            // 有空闲的线程，通知等待中的一个线程过来取任务并执行
            condition.notify_one();
        }
        else
        {
            // 无空闲的线程，就同步执行
            std::unique_lock<std::mutex> lock(task_mtx);
            auto task_tmp = std::move(tasks.front());
            tasks.pop();
            lock.unlock();
            task_tmp();
        }

        return future;
    }

private:
    std::vector<std::thread> threads;           // 线程容器
    std::vector<bool> busyFlags;   // 每个线程的繁忙状态标志
    std::queue<std::function<void()>> tasks;    // 任务队列
    std::mutex task_mtx;                        // 任务的互斥锁
    std::mutex active_threads_mtx;              // 线程数计算的互斥锁
    std::condition_variable condition;          // 条件变量
    std::thread management_thread;              // 管理线程
    bool stop;                                  // 停止标志
    int min_threads;                            // 最小的线程数量
    int max_threads;                            // 最大的线程数量
    int active_threads;                         // 用于记录正在执行任务的线程数

    // 判断是否有可用线程
    bool HasAvailableThreads() const
    {
        return active_threads < threads.size();
    }

    // 创建一个线程并且从任务队列中读取任务
    void CreateThread()
    {
        busyFlags.emplace_back(false);
        threads.emplace_back([this](size_t thread_index) {
            while (true)
            {
                std::unique_lock<std::mutex> lock(task_mtx);
                // 等待任务队列不为空或线程池停止
                condition.wait(lock, [this](){
                    return !tasks.empty() || stop;
                });

                // 线程池停止且任务队列不为空的时候返回
                if (stop && tasks.empty()) return;

                // 设置繁忙状态
                busyFlags[thread_index] = true;

                // 任务开始，增加活跃线程数
                {
                    std::unique_lock<std::mutex> lock(active_threads_mtx);
                    ++active_threads;
                }

                //从任务队列中取出任务并完成完成
                auto task = std::move(tasks.front());
                tasks.pop();
                lock.unlock();
                task();
                    
                // 任务完成，减少活跃线程数
                {
                    std::unique_lock<std::mutex> lock(active_threads_mtx);
                    --active_threads;
                }

                // 重置繁忙状态
                busyFlags[thread_index] = false;
            }
        }, busyFlags.size() - 1);// 传递线程编号作为参数
    }

    void RecycleThreads()
    {
        std::unique_lock<std::mutex> lock(active_threads_mtx);

        // 遍历每个线程的繁忙状态
        for (size_t i = 0; i < busyFlags.size(); ++i)
        {
            if (!busyFlags[i])
            {
                //线程不繁忙，可以删除
                if (threads[i].joinable())
                {
                    threads[i].join();
                }
                threads.erase(threads.begin() + i);
                busyFlags.erase(busyFlags.begin() + i);
                --i; // 因为删除了一个元素，所以下一个元素的索引减1
            }
        }
    }

    void ExpandThreads(int num)
    {
        for(int i = 0; i < num; ++i)
        {
            CreateThread();
        }
    }
};

// 时间测试函数
template <typename Func, typename... Args>
auto MeasureTime(Func func, Args... args)
{
    auto start_time = std::chrono::high_resolution_clock::now();

    // 调用传入的函数并且传递参数
    auto result = func(args...);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "运行时间为：" << duration.count() << "ms" << std::endl;

    return std::make_pair(result, duration.count());
}

#endif