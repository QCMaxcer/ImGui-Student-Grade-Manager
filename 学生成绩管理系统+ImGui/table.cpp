#include "table.h"
#include <iostream>
using namespace std;
void Table::refreshTable()
{
    // ˢ��ѧ���б�
    table.table_students.clear();
    table.table_students.shrink_to_fit();
    // ����ѧ���б�
    for (const auto& student : manager.students)
    {
        //���� ������ѧ�Ŷ�������Ͳ������б�
        if (strlen(table.filter.searchBuffer) > 0)
        {
            if (strstr(student.name.c_str(), table.filter.searchBuffer) == nullptr &&
                std::to_string(student.id).find(table.filter.searchBuffer) == std::string::npos) {
                continue;
            }
        }
        //����ȱ�����������б� ȱ���������γ̳ɼ�Ϊ-1
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
        //���˼���/������
        bool show = true;
        for (int i = 0; i < manager.courses.size() + 1; i++) //ѭ���ɼ�
        {
            if (i == manager.courses.size()) //�ܷ�
            {
                if (table.filter.course_filters[0].showFailed) //��ʾ������/����
                {
                    if (student.totalScore >= table.filter.course_filters[0].passLine) //������
                    {
                        if (table.filter.course_filters[0].showFailed == 1) //ֻ��ʾ��������
                        {
                            show = false;
                            break;
                        }
                    }
                    else //��������
                    {
                        if (table.filter.course_filters[0].showFailed == 2) //ֻ��ʾ������
                        {
                            show = false;
                            break;
                        }
                    }
                }
                //goto end;
            }
            else //�γ̷�
            {
                if (table.filter.course_filters[i + 1].showFailed) //��ʾ������/����
                {
                    if (student.scores[i] >= table.filter.course_filters[i + 1].passLine) //������
                    {
                        if (table.filter.course_filters[i + 1].showFailed == 1) //ֻ��ʾ��������
                        {
                            show = false;
                            break;
                        }
                    }
                    else //��������
                    {
                        if (table.filter.course_filters[i + 1].showFailed == 2) //ֻ��ʾ������
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

//// ���ѧ���Ƿ������������
//bool matchSearchFilter(const Student& student, const char* searchBuffer) {
//    return strlen(searchBuffer) == 0 ||
//        strstr(student.name.c_str(), searchBuffer) != nullptr ||
//        std::to_string(student.id).find(searchBuffer) != std::string::npos;
//}
//
//// ���ѧ���Ƿ����ȱ������
//bool matchAbsentFilter(const Student& student, bool showAbsent, const Manager& manager) {
//    if (!showAbsent) return true;
//    for (int score : student.scores) {
//        if (score == -1) return true;
//    }
//    return false;
//}
//
//// ���ѧ���Ƿ���ϼ���/����������
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
//    // ˢ��ѧ���б�
//    table.table_students.clear();
//    table.table_students.shrink_to_fit();
//    // ����ѧ���б�
//    for (const auto& student : manager.students) {
//        if (!matchSearchFilter(student, table.filter.searchBuffer)) continue;
//        if (!matchAbsentFilter(student, table.filter.showAbsent, manager)) continue;
//        if (!matchPassFilter(student, table, manager)) continue;
//
//        table.table_students.push_back(student);
//    }
//}
