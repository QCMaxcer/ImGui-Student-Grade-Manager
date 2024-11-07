#pragma once
#include "student.h"
#include <deque>
enum ActionType {
    AddStudent,
    DeleteStudent,
    ModifyStudent,
    AddCourse,
    DeleteCourse,
    ModifyCourse
};

struct Action {
    ActionType type;
    Student studentData;
    Student newStudentData;
    vector<float> courseData;
    string courseName;
    string newCourseName;
};


struct History {
    //int maxHistorySize = 64;
    std::deque<Action> history;  // 操作历史的 deque
    int currentHistoryIndex = 0;  // 撤销/还原的指针
    void addAction(ActionType type, Student studentData); //添加学生 + 删除学生
    void addAction(ActionType type, Student studentData, Student newStudentData); //修改学生
    void addAction(ActionType type, string courseName, string newCourseName); //添加课程 + 修改课程
    void addAction(ActionType type, vector<float> courseData, string courseName); //删除课程
    void undo();
    void redo();
    void clear();
    bool getCanUndo();
    bool getCanRedo();
};

extern History history;