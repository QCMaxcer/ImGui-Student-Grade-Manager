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
    cv.notify_one();  // ֪ͨ�߳�״̬�Ѹı�
}

void setAutoSaveInterval(int minutes)
{
    setting.autoSaveInterval = std::chrono::minutes(minutes);
    cv.notify_one();  // ֪ͨ�̼߳��ʱ���Ѹ���
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
        // �ȴ�ָ��ʱ����Զ���������״̬�ı�
        cv.wait_for(lock, setting.autoSaveInterval /*std::chrono::seconds(5)*/, [] { return !setting.autoSaveEnabled.load() || stop.load(); });

        if (stop) {
            break; // ����˳���־Ϊ�棬��ֱ���˳�ѭ��
        }

        if (setting.autoSaveEnabled && !setting.cur_file_path.empty()) {
            manager.saveToFile();
            //cout << "Auto saved" << endl;
        }

        else {
            // ����Զ����汻���ã��ȴ���һ������
            cv.wait(lock, [] { return setting.autoSaveEnabled.load(); });
        }
    }
}