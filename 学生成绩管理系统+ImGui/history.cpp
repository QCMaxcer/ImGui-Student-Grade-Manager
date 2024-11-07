#include "history.h"
#include "manager.h"
#include "setting.h"
#include "table.h"
void History::addAction(ActionType type, Student studentData)
{
    // 清除当前指针之后的历史记录
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }

    // 创建新的操作并添加到 deque 中
    Action action = { type, studentData, studentData };
    history.push_back(action);

    if (history.size() > setting.maxHistorySize) {
        history.pop_front();
    }
    else {
        currentHistoryIndex++;
    }
}

void History::addAction(ActionType type, Student studentData, Student newStudentData)
{
    // 清除当前指针之后的历史记录
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }

    // 创建新的操作并添加到 deque 中
    Action action = { type, studentData, newStudentData };  //撤消修改的学生信息
    history.push_back(action);

    if (history.size() > setting.maxHistorySize) {
        history.pop_front();
    }
    else {
        currentHistoryIndex++;
    }
}

void History::addAction(ActionType type, string courseName, string newCourseName)
{
    // 清除当前指针之后的历史记录
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }
    vector<float> score;
    Student studentData(NULL, "", score);
    // 创建新的操作并添加到 deque 中
    Action action = { type, studentData, studentData, score, courseName , newCourseName }; //撤消添加的课程
    history.push_back(action);

    if (history.size() > setting.maxHistorySize) {
        history.pop_front();
    }
    else {
        currentHistoryIndex++;
    }
}

void History::addAction(ActionType type, vector<float> courseData, string courseName)
{
    // 清除当前指针之后的历史记录
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }
    vector<float> score;
    Student studentData(NULL, "", score);
    // 创建新的操作并添加到 deque 中
    Action action = { type, studentData, studentData, courseData, courseName, courseName };
    history.push_back(action);

    if (history.size() > setting.maxHistorySize) {
        history.pop_front();
    }
    else {
        currentHistoryIndex++;
    }
}

void History::undo()
{
    if (currentHistoryIndex == 0)
    {
        return;
    }

    currentHistoryIndex--;
    Action action = history[currentHistoryIndex];

    switch (action.type) {
    case ActionType::AddStudent:
        manager.deleteStudent(action.studentData.id, false);
        break;
    case ActionType::DeleteStudent:
        manager.addStudent(action.studentData, false);
        break;
    case ActionType::ModifyStudent:
        manager.modifyStudent(action.studentData, false);
        break;
    case ActionType::AddCourse:
        manager.deleteCourse(action.courseName, false);
        break;
    case ActionType::DeleteCourse:
        manager.addCourse(action.courseName, action.courseData, false);
        break;
    case ActionType::ModifyCourse:
        manager.modifyCourse(action.newCourseName, action.courseName, false);
        break;
    default:
        break;
    }
    table.student_changed = true; // 通知 table 更新数据
}

bool History::getCanUndo()
{
    return currentHistoryIndex > 0;
}

void History::redo()
{
    if (currentHistoryIndex == history.size()) {
        return;
    }

    Action action = history[currentHistoryIndex];
    currentHistoryIndex++;

    switch (action.type) {
    case ActionType::AddStudent:
        manager.addStudent(action.studentData, false);
        break;
    case ActionType::DeleteStudent:
        manager.deleteStudent(action.studentData.id, false);
        break;
    case ActionType::ModifyStudent:
        manager.modifyStudent(action.newStudentData, false);
        break;
    case ActionType::AddCourse:
        manager.addCourse(action.courseName, false);
        break;
    case ActionType::DeleteCourse:
        manager.deleteCourse(action.courseName, false);
        break;
    case ActionType::ModifyCourse:
        manager.modifyCourse(action.courseName, action.newCourseName, false);
        break;
    default:
        break;
    }
    table.student_changed = true; // 通知 table 更新数据
}

bool History::getCanRedo()
{
    return currentHistoryIndex < history.size();
}

void History::clear()
{
    history.clear();
    currentHistoryIndex = 0;
}