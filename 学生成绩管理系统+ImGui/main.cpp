#include "ui.h"
#include "manager.h"
#include "setting.h"
#include "client.h"
#include "table.h"
#include "history.h"
#include "autoSave.h"
#include <iostream>
using namespace std;

Setting setting;
Client client;
Manager manager;
Table table;
History history;

void initProgram()
{
    std::wcout.imbue(std::locale("chs"));

    client.show_load_error_window = manager.loadFromFile(true);
}

//
//void printManager()
//{
//    //从打印manager中的学生数据
//    for (int i = 0; i < manager.students.size(); i++)
//    {
//        cout << "学生" << i + 1 << "：" << manager.students[i].getName_str() << "的成绩如下：" << endl;
//        for (int j = 0; j < manager.courses.size(); j++)
//        {
//            cout << manager.getCourseName_str(j) << "： " << manager.students[i].scores[j] << " ";
//        }
//        cout << endl;
//    }
//    //打印各科平均分
//    cout << "各科平均分：" << endl;
//    for (int i = 0; i < manager.courses.size(); i++)
//    {
//        cout << manager.getCourseName_str(i) << "： " << manager.averageScores[i] << " ";
//    }
//    cout << endl;
//}

// Main code
//int main(int, char**)
//窗口 (/SUBSYSTEM:WINDOWS)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   initProgram();

    std::thread autoSaveThread(autoSave);
    ui();


    stopAutoSave();
    autoSaveThread.join();
    return 0;
}
