#include "manager.h"
#include "setting.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <iostream>

std::condition_variable cv;
std::mutex mtx;
std::atomic<bool> stop = false;

using namespace std;
void toggleAutoSave(bool enable)
{
    setting.autoSaveEnabled = enable;
    cv.notify_one();  // 通知线程状态已改变
}

void setAutoSaveInterval(int minutes)
{
    setting.autoSaveInterval = std::chrono::minutes(minutes);
    cv.notify_one();  // 通知线程间隔时间已更新
}

void stopAutoSave()
{
    stop = true;
    cv.notify_one();
}

void autoSave()
{
    std::unique_lock<std::mutex> lock(mtx);

    while (true) {
        // 等待指定时间或自动保存启用状态改变
        cv.wait_for(lock, setting.autoSaveInterval /*std::chrono::seconds(5)*/, [] { return !setting.autoSaveEnabled.load() || stop.load(); });

        if (stop) {
            break; // 如果退出标志为真，则直接退出循环
        }

        if (setting.autoSaveEnabled && !setting.cur_file_path.empty()) {
            manager.saveToFile();
            //cout << "Auto saved" << endl;
        }

        else {
            // 如果自动保存被禁用，等待下一次启用
            cv.wait(lock, [] { return setting.autoSaveEnabled.load(); });
        }
    }
}