#include "manager.h"
#include "history.h"
#include "table.h"
#include "setting.h"
#include "utf8_to_string.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

Manager::Manager()
{
    //loadFromFile();
}

string Manager::getCourseName_str(int index)
{
    return utf8_to_string(courses[index]);
}

void Manager::calculateAverageScore()
{
    totalAverageScore = 0;
    if (students.empty())
    {
        //totalAverageScore = 0;
        //�����п�Ŀ��ƽ������Ϊ0
        //averageScores.resize(courses.size());
        for (int i = 0; i < courses.size(); i++)
        {
            averageScores[i] = 0;
        }
        return;
    }
    //����ÿ��ѧ����ƽ����
    for (int i = 0; i < courses.size(); i++)
    {
        double sum = 0;
        for (int j = 0; j < students.size(); j++)
        {
            if(students[j].scores[i] == -1)
                continue;
            sum += students[j].scores[i];
        }
        double average = sum / students.size();
        averageScores[i] = average;
        totalAverageScore += average;
    }
    ////�����ֵܷ�ƽ����
    //double sum = 0;
    //for (int i = 0; i < students.size(); i++)
    //{
    //    sum += averageScores[i];
    //}
    //totalAverageScore = sum;
}

//����calculateAverageScore��
//1. �����ж�ѧ���б��Ƿ�Ϊ�գ����Ϊ�գ������п�Ŀ��ƽ������Ϊ0
//2. ��������ѧ��������ÿ��ѧ�����ܷ֣������ֳܷ���ѧ���������õ�ÿ��ѧ����ƽ����
//3. �������пγ̣�����ÿ���γ̵�ƽ���֣�����ƽ���ֱ��浽averageScores�б���
//4. �����ֵܷ�ƽ���֣������䱣�浽totalAverageScore��

void Manager::newFile()
{
    setting.cur_file_path.clear();
    setting.cur_file_name.clear();
    setting.saveSetting();
    clearStudents();
    clearCourses();
}

void Manager::save()
{
    history.clear();
    ofstream file(setting.cur_file_path);
    if (!file.is_open())
    {
        //cout << "open file failed" << endl;
        return;
    }
    file << "courses:" << endl;
    for (int i = 0; i < courses.size(); i++)
    {
        file << courses[i] << endl;
    }
    file << "students:" << endl;
    for (int i = 0; i < students.size(); i++)
    {
        file << students[i].id << " " << students[i].name << " ";
        for (int j = 0; j < courses.size(); j++)
        {
            file << students[i].scores[j] << " ";
        }
        file << endl;
    }
    file.close();
}
void Manager::saveToFile()
{
    if (setting.cur_file_name == L"")
    {
        setting.cur_file_path = setting.saveFileDialog();
        if (setting.cur_file_path == L"")
        {
            return;
        }
        setting.cur_file_name = setting.cur_file_path.substr(setting.cur_file_path.find_last_of(L"/\\") + 1);
        setting.saveSetting();
    }
    save();
}

void Manager::saveToFileAs()
{
    wstring new_file_path = setting.saveFileDialog();
    if (new_file_path == L"")
    {
        return;
    }
    setting.cur_file_path = new_file_path;
    setting.cur_file_name = setting.cur_file_path.substr(setting.cur_file_path.find_last_of(L"/\\") + 1);
    setting.saveSetting();
    save();
}

int Manager::loadFromFile(bool firstLoad)
{
    if (!firstLoad)
    {
        setting.cur_file_path = setting.openFileDialog();
        if (setting.cur_file_path == L"")
        {
            return 0;
        }
        setting.cur_file_name = setting.cur_file_path.substr(setting.cur_file_path.find_last_of(L"/\\") + 1);
        setting.saveSetting();
    }
    else
    {
        if (setting.cur_file_path == L"")
        {
            newFile();
            return 0;
        }
    }

    ifstream file(setting.cur_file_path);
    if (!file.is_open())
    {
        if (firstLoad)
        {
            newFile();
        }
        //cout << "open file failed" << endl;
        return 1;
    }
    string line;
    getline(file, line);
    if (line != "courses:")
    {
        if (firstLoad)
        {
            newFile();
        }
        //cout << "file format error" << endl;
        return 2; //�ļ���ʽ����
    }
    clearStudents(); //�����ԭ�ȵ�ѧ����Ϣ
    clearCourses(); //�����ԭ�ȵĿγ���Ϣ
    while (getline(file, line)) //��ȡ�γ���Ϣ
    {
        if (line == "students:")
        {
            break;
        }
        addCourse(line, false);
    }
    while (getline(file, line)) //��ȡѧ����Ϣ
    {
        stringstream ss(line);
        long long id;
        string name;
        vector<float> scores;
        ss >> id >> name;
        for (int i = 0; i < courses.size(); i++)
        {
            float score;
            ss >> score;
            scores.push_back(score);
        }
        Student student(id, name, scores);
        addStudent(student, false);
    }
    file.close();
    calculateAverageScore();
    table.filter.initFilter();
    table.refreshTable();
    return 0;
}



int Manager::addCourse(string course, bool undoable)
{
    if (course.empty())
    {
        return 1; //�γ���Ϊ��
    }
    //���γ��Ƿ��Ѿ�����
    for (int i = 0; i < courses.size(); i++)
    {
        if (courses[i] == course)
        {
            return 2; //�γ��Ѿ�����
        }
    }
    for (int i = 0; i < students.size(); i++)
    {
        students[i].scores.push_back(-1.0f);
    }
    courses.push_back(course);
    averageScores.push_back(0.0f); //���ƽ����
    Course_filter filter;
    table.filter.course_filters.push_back(filter); //��ӹ�����
    if(undoable)
        history.addAction(AddCourse, course, course);
    //calculateAverageScore();
    return 0;
}

//����addCourse��
//1. �����жϿγ����Ƿ�Ϊ��
//2. Ȼ���������ѧ����Ϊÿ��ѧ�����һ��-1�ķ������Ա������ӿγ�ʱ��ÿ��ѧ�����ж�Ӧ�ķ���
//3. ��γ��б�����ӿγ�
//4. ��ƽ�����б������0.0f
//5. ��������б������һ���յĹ�����
//6. ����history.addAction()��������¼����

void Manager::addCourse(string course, vector<float> courseScores, bool undoable)
{
    courses.push_back(course);

    for (int i = 0; i < students.size(); i++)
    {
        students[i].scores.push_back(courseScores[i]);
        students[i].calculateTotalScore();
    }
    averageScores.push_back(0.0f); //���ƽ����
    Course_filter filter;
    table.filter.course_filters.push_back(filter); //��ӹ�����
    if (undoable)
        history.addAction(AddCourse, course, course);
    calculateAverageScore();
}
//���ڻ�ԭ������addCouse��������

void Manager::deleteCourse(string course, bool undoable)
{
    vector <float> courseScores;
    int index = -1;
    for (int i = 0; i < courses.size(); i++)
    {
        if (courses[i] == course)
        {
            index = i;
            break;
        }
    } //�ҵ�Ҫɾ���Ŀγ̵�����
    for (int i = 0; i < students.size(); i++)
    {
        courseScores.push_back(students[i].scores[index]); //����ԭ��ѧ���ķ���
        students[i].scores.erase(students[i].scores.begin() + index); //ɾ��ԭ��ѧ���ķ���
        students[i].calculateTotalScore();//�����ܷ�
    }
    averageScores.erase(averageScores.begin() + index); //ɾ��ƽ����
    table.filter.course_filters.erase(table.filter.course_filters.begin() + index + 1); //ɾ��������
    courses.erase(courses.begin() + index); //ɾ���γ�
    if(undoable)
        history.addAction(DeleteCourse, courseScores, course);
    calculateAverageScore();
}

//����deleteCourse��
//1. ���ȱ�������ѧ��������ԭ��ѧ���ķ���
//2. Ȼ��ɾ���γ�
//3. ����calculateAverageScore()����������ƽ����
//4. ����history.addAction()��������¼����

int Manager::modifyCourse(string course, string newCourse, bool undoable)
{
    if (newCourse.empty())
    {
        return 1; //�γ���Ϊ��
    }
    //����޸������Ƿ���ԭ������ͬ
    if (course == newCourse)
    {
        return 2;
    }
    //���γ��Ƿ��Ѿ�����
    for (int i = 0; i < courses.size(); i++)
    {
        if (courses[i] == newCourse)
        {
            return 3; //�γ��Ѿ�����
        }
    }
    int index = -1;
    for (int i = 0; i < courses.size(); i++)
    {
        if (courses[i] == course)
        {
            index = i;
            break;
        }
    }
    courses[index] = newCourse;
    if(undoable)
        history.addAction(ModifyCourse, course, newCourse);
    //calculateAverageScore();
    return 0;
}

//����modifyCourse��
//1. �����ж��¿γ����Ƿ�Ϊ��
//2. Ȼ��������пγ̣��ҵ�Ҫ�޸ĵĿγ̵�����
//3. �޸Ŀγ���
//4. ����history.addAction()��������¼����

void Manager::clearCourses()
{
    courses.clear();
    table.filter.course_filters.resize(1);
    for (int i = 0; i < students.size(); i++)
    {
        students[i].scores.clear();
        students[i].totalScore = 0;
    }
    totalAverageScore = 0.0f;
    averageScores.clear();
    history.clear();
}

//����clearCourses��
//1. ������տγ��б�
//2. Ȼ����չ������б�
//3. Ȼ�����ѧ���ķ����б�
//4. Ȼ�����ƽ�����б�
//5. ����calculateAverageScore()����������ƽ����
//6. ����history.clear()��������ղ�����¼

int Manager::addStudent(Student student, bool undoable)
{
    if (student.id < 0)
    {
        return 4; //ѧ�Ų��Ϸ�
    }
    if (student.name.empty())
    {
        return 3; //����Ϊ��
    }
    //���ѧ���Ƿ��Ѿ�����
    for (int i = 0; i < students.size(); i++) 
    {
        if (students[i].id == student.id) 
        {
            return 1; //ѧ���ظ�
        }
    }
    for (int i = 0; i < courses.size(); i++) 
    {
        float score = student.scores[i];
        if (score < 0.0f && score != -1.0f)
        {
            return 2; //�������Ϸ�
        }
    }
    students.push_back(student);
    if(undoable)
        history.addAction(AddStudent, student);
    calculateAverageScore();
    return 0;
}

//����addStudent��
//1. ���ȼ��ѧ���Ƿ��ظ�
//2. Ȼ���������Ƿ�Ϊ��
//3. Ȼ��������Ƿ�Ϸ�
//4. Ȼ����ѧ���б������ѧ��
//5. ����calculateAverageScore()����������ƽ����
//6. ����history.addAction()��������¼����

void Manager::deleteStudent(int id, bool undoable)
{
    for (int i = 0; i < students.size(); i++) 
    {
        if (students[i].id == id) 
        {
            if(undoable)
                history.addAction(DeleteStudent, students[i]);
            students.erase(students.begin() + i);
            calculateAverageScore();
            return;
        }
    }
}

//����deleteStudent��
//1. ���ȱ���ѧ���б��ҵ�Ҫɾ����ѧ��������
//2. Ȼ�����history.addAction()��������¼����
//3. Ȼ��ɾ��ѧ��
//4. ����calculateAverageScore()����������ƽ����

int Manager::modifyStudent(Student student, bool undoable)
{
    //���ȼ�������Ƿ�Ϊ��
    if (student.name.empty())
    {
        return 3; //����Ϊ��
    }
    for (int i = 0; i < courses.size(); i++)
    {
        if (student.scores[i] < -1.0f || (student.scores[i] > -1.0f && student.scores[i] < 0.0f))
        {
            return 2; //�������Ϸ�
        }
    }

    for (int i = 0; i < students.size(); i++) 
    {
        if (students[i].id == student.id)
        {
            if (undoable)
                history.addAction(ModifyStudent, students[i], student);
            students[i] = student;
            calculateAverageScore();
            return 0;
        }
    }
}

//����modifyStudent��
//1. ���ȼ�������Ƿ�Ϊ��
//2. Ȼ��������Ƿ�Ϸ�
//3. Ȼ�����ѧ���б��ҵ�Ҫ�޸ĵ�ѧ��������
//4. Ȼ���޸�ѧ����Ϣ
//5. ����calculateAverageScore()����������ƽ����
//6. ����history.addAction()��������¼����

void Manager::clearStudents()
{
    students.clear();
    calculateAverageScore();
    history.clear();
}

//����clearStudents��
//1. �������ѧ���б�
//2. Ȼ�����calculateAverageScore()����������ƽ����
//3. ����history.clear()��������ղ�����¼

void Manager::insertRandom(int stu_count)
{
    newFile();
    courses.push_back(u8"����");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"��ѧ");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"Ӣ��");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"����");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"��ѧ");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"����");
    averageScores.push_back(-1.0f);

    random_device rd;   // ʹ��Ӳ����������ӣ�������ã�
    mt19937 gen(rd());  // Mersenne Twister 19937 ����
    uniform_int_distribution<> distrib(-1, 100);  // ���� 1 �� 100 ֮����������

    for (int i = 0; i < stu_count; i++) //���stu_count��ѧ��
    {
        vector<float> scores;
        for (int j = 0; j < manager.courses.size(); j++)
        {
            scores.push_back(distrib(gen));
        }
        Student student(i + 1, u8"ѧ��" + to_string(i + 1), scores);
        students.push_back(student);
    }

    calculateAverageScore();
    table.filter.initFilter();
    table.refreshTable();
}