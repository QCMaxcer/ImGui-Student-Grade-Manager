#pragma once
#include "manager.h"
struct Course_filter
{
    int showFailed = 0;  // 0: show all, 1: show failed, 2: show passed
    bool markFailed = false;
    float passLine = 60.0f;
};

struct Filter
{
    bool markAbsent = false;
    bool showAbsent = false;
    char searchBuffer[128] = "";
    vector<Course_filter> course_filters;
    void initFilter()
    {
        Course_filter cf;
        course_filters.assign(manager.courses.size() + 1, cf);
        course_filters[0].passLine = 60.0f * manager.courses.size();
    }
};

struct Table
{
    Filter filter;
    vector<Student> table_students;
    bool student_changed = false;
    void refreshTable();
};

extern Table table;