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
    std::deque<Action> history;  // ������ʷ�� deque
    int currentHistoryIndex = 0;  // ����/��ԭ��ָ��
    void addAction(ActionType type, Student studentData); //���ѧ�� + ɾ��ѧ��
    void addAction(ActionType type, Student studentData, Student newStudentData); //�޸�ѧ��
    void addAction(ActionType type, string courseName, string newCourseName); //��ӿγ� + �޸Ŀγ�
    void addAction(ActionType type, vector<float> courseData, string courseName); //ɾ���γ�
    void undo();
    void redo();
    void clear();
    bool getCanUndo();
    bool getCanRedo();
};

extern History history;