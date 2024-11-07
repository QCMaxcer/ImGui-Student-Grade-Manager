#include "history.h"
#include "manager.h"
#include "setting.h"
#include "table.h"
void History::addAction(ActionType type, Student studentData)
{
    // �����ǰָ��֮�����ʷ��¼
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }

    // �����µĲ�������ӵ� deque ��
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
    // �����ǰָ��֮�����ʷ��¼
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }

    // �����µĲ�������ӵ� deque ��
    Action action = { type, studentData, newStudentData };  //�����޸ĵ�ѧ����Ϣ
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
    // �����ǰָ��֮�����ʷ��¼
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }
    vector<float> score;
    Student studentData(NULL, "", score);
    // �����µĲ�������ӵ� deque ��
    Action action = { type, studentData, studentData, score, courseName , newCourseName }; //������ӵĿγ�
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
    // �����ǰָ��֮�����ʷ��¼
    while (history.size() > currentHistoryIndex) {
        history.pop_back();
    }
    vector<float> score;
    Student studentData(NULL, "", score);
    // �����µĲ�������ӵ� deque ��
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
    table.student_changed = true; // ֪ͨ table ��������
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
    table.student_changed = true; // ֪ͨ table ��������
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