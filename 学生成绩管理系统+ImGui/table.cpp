#include "table.h"
#include <iostream>
using namespace std;
void Table::refreshTable()
{
    // 刷新学生列表
    table.table_students.clear();
    table.table_students.shrink_to_fit();
    // 过滤学生列表
    for (const auto& student : manager.students)
    {
        //搜索 姓名或学号都不相符就不加入列表
        if (strlen(table.filter.searchBuffer) > 0)
        {
            if (strstr(student.name.c_str(), table.filter.searchBuffer) == nullptr &&
                std::to_string(student.id).find(table.filter.searchBuffer) == std::string::npos) {
                continue;
            }
        }
        //不是缺考生不加入列表 缺考条件：课程成绩为-1
        if (table.filter.showAbsent)
        {
            bool hasAbsent = false;
            for (int i = 0; i < manager.courses.size(); i++)
            {
                if (student.scores[i] == -1)
                {
                    hasAbsent = true;
                    break;
                }
            }
            if (!hasAbsent)
            {
                continue;
            }
        }
        //过滤及格/不及格
        bool show = true;
        for (int i = 0; i < manager.courses.size() + 1; i++) //循环成绩
        {
            if (i == manager.courses.size()) //总分
            {
                if (table.filter.course_filters[0].showFailed) //显示不及格/及格
                {
                    if (student.totalScore >= table.filter.course_filters[0].passLine) //及格生
                    {
                        if (table.filter.course_filters[0].showFailed == 1) //只显示不及格生
                        {
                            show = false;
                            break;
                        }
                    }
                    else //不及格生
                    {
                        if (table.filter.course_filters[0].showFailed == 2) //只显示及格生
                        {
                            show = false;
                            break;
                        }
                    }
                }
                //goto end;
            }
            else //课程分
            {
                if (table.filter.course_filters[i + 1].showFailed) //显示不及格/及格
                {
                    if (student.scores[i] >= table.filter.course_filters[i + 1].passLine) //及格生
                    {
                        if (table.filter.course_filters[i + 1].showFailed == 1) //只显示不及格生
                        {
                            show = false;
                            break;
                        }
                    }
                    else //不及格生
                    {
                        if (table.filter.course_filters[i + 1].showFailed == 2) //只显示及格生
                        {
                            show = false;
                            break;
                        }
                    }
                }
            }
        }
    //end:
        if (!show)
        {
            continue;
        }
        table.table_students.push_back(student);
    }
}

//// 检查学生是否符合搜索条件
//bool matchSearchFilter(const Student& student, const char* searchBuffer) {
//    return strlen(searchBuffer) == 0 ||
//        strstr(student.name.c_str(), searchBuffer) != nullptr ||
//        std::to_string(student.id).find(searchBuffer) != std::string::npos;
//}
//
//// 检查学生是否符合缺考条件
//bool matchAbsentFilter(const Student& student, bool showAbsent, const Manager& manager) {
//    if (!showAbsent) return true;
//    for (int score : student.scores) {
//        if (score == -1) return true;
//    }
//    return false;
//}
//
//// 检查学生是否符合及格/不及格条件
//bool matchPassFilter(const Student& student, const Table& table, const Manager& manager) {
//    for (int i = 0; i <= manager.courses.size(); i++) {
//        int score = (i == manager.courses.size()) ? student.totalScore : student.scores[i];
//        const auto& filter = table.filter.course_filters[i];
//
//        if (filter.showFailed) {
//            bool isPassing = score >= filter.passLine;
//            if ((isPassing && filter.showFailed == 1) || (!isPassing && filter.showFailed == 2)) {
//                return false;
//            }
//        }
//    }
//    return true;
//}
//void Table::refreshTable()
//{
//    // 刷新学生列表
//    table.table_students.clear();
//    table.table_students.shrink_to_fit();
//    // 过滤学生列表
//    for (const auto& student : manager.students) {
//        if (!matchSearchFilter(student, table.filter.searchBuffer)) continue;
//        if (!matchAbsentFilter(student, table.filter.showAbsent, manager)) continue;
//        if (!matchPassFilter(student, table, manager)) continue;
//
//        table.table_students.push_back(student);
//    }
//}
