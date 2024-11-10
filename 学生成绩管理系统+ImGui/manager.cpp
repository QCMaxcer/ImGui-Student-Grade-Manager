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
        //将所有科目的平均分置为0
        //averageScores.resize(courses.size());
        for (int i = 0; i < courses.size(); i++)
        {
            averageScores[i] = 0;
        }
        return;
    }
    //计算每个学生的平均分
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
    ////计算总分的平均分
    //double sum = 0;
    //for (int i = 0; i < students.size(); i++)
    //{
    //    sum += averageScores[i];
    //}
    //totalAverageScore = sum;
}

//解释calculateAverageScore：
//1. 首先判断学生列表是否为空，如果为空，则将所有科目的平均分置为0
//2. 遍历所有学生，计算每个学生的总分，并将总分除以学生数量，得到每个学生的平均分
//3. 遍历所有课程，计算每个课程的平均分，并将平均分保存到averageScores列表中
//4. 计算总分的平均分，并将其保存到totalAverageScore中

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
        return 2; //文件格式错误
    }
    clearStudents(); //先清空原先的学生信息
    clearCourses(); //先清空原先的课程信息
    while (getline(file, line)) //读取课程信息
    {
        if (line == "students:")
        {
            break;
        }
        addCourse(line, false);
    }
    while (getline(file, line)) //读取学生信息
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
        return 1; //课程名为空
    }
    //检测课程是否已经存在
    for (int i = 0; i < courses.size(); i++)
    {
        if (courses[i] == course)
        {
            return 2; //课程已经存在
        }
    }
    for (int i = 0; i < students.size(); i++)
    {
        students[i].scores.push_back(-1.0f);
    }
    courses.push_back(course);
    averageScores.push_back(0.0f); //添加平均分
    Course_filter filter;
    table.filter.course_filters.push_back(filter); //添加过滤器
    if(undoable)
        history.addAction(AddCourse, course, course);
    //calculateAverageScore();
    return 0;
}

//解释addCourse：
//1. 首先判断课程名是否为空
//2. 然后遍历所有学生，为每个学生添加一个-1的分数，以便后面添加课程时，每个学生都有对应的分数
//3. 向课程列表中添加课程
//4. 向平均分列表中添加0.0f
//5. 向过滤器列表中添加一个空的过滤器
//6. 调用history.addAction()函数，记录操作

void Manager::addCourse(string course, vector<float> courseScores, bool undoable)
{
    courses.push_back(course);

    for (int i = 0; i < students.size(); i++)
    {
        students[i].scores.push_back(courseScores[i]);
        students[i].calculateTotalScore();
    }
    averageScores.push_back(0.0f); //添加平均分
    Course_filter filter;
    table.filter.course_filters.push_back(filter); //添加过滤器
    if (undoable)
        history.addAction(AddCourse, course, course);
    calculateAverageScore();
}
//用于还原操作的addCouse函数重载

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
    } //找到要删除的课程的索引
    for (int i = 0; i < students.size(); i++)
    {
        courseScores.push_back(students[i].scores[index]); //保存原先学生的分数
        students[i].scores.erase(students[i].scores.begin() + index); //删除原先学生的分数
        students[i].calculateTotalScore();//计算总分
    }
    averageScores.erase(averageScores.begin() + index); //删除平均分
    table.filter.course_filters.erase(table.filter.course_filters.begin() + index + 1); //删除过滤器
    courses.erase(courses.begin() + index); //删除课程
    if(undoable)
        history.addAction(DeleteCourse, courseScores, course);
    calculateAverageScore();
}

//解释deleteCourse：
//1. 首先遍历所有学生，保存原先学生的分数
//2. 然后删除课程
//3. 调用calculateAverageScore()函数，计算平均分
//4. 调用history.addAction()函数，记录操作

int Manager::modifyCourse(string course, string newCourse, bool undoable)
{
    if (newCourse.empty())
    {
        return 1; //课程名为空
    }
    //检测修改名称是否与原名称相同
    if (course == newCourse)
    {
        return 2;
    }
    //检测课程是否已经存在
    for (int i = 0; i < courses.size(); i++)
    {
        if (courses[i] == newCourse)
        {
            return 3; //课程已经存在
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

//解释modifyCourse：
//1. 首先判断新课程名是否为空
//2. 然后遍历所有课程，找到要修改的课程的索引
//3. 修改课程名
//4. 调用history.addAction()函数，记录操作

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

//解释clearCourses：
//1. 首先清空课程列表
//2. 然后清空过滤器列表
//3. 然后清空学生的分数列表
//4. 然后清空平均分列表
//5. 调用calculateAverageScore()函数，计算平均分
//6. 调用history.clear()函数，清空操作记录

int Manager::addStudent(Student student, bool undoable)
{
    if (student.id < 0)
    {
        return 4; //学号不合法
    }
    if (student.name.empty())
    {
        return 3; //姓名为空
    }
    //检测学号是否已经存在
    for (int i = 0; i < students.size(); i++) 
    {
        if (students[i].id == student.id) 
        {
            return 1; //学号重复
        }
    }
    for (int i = 0; i < courses.size(); i++) 
    {
        float score = student.scores[i];
        if (score < 0.0f && score != -1.0f)
        {
            return 2; //分数不合法
        }
    }
    students.push_back(student);
    if(undoable)
        history.addAction(AddStudent, student);
    calculateAverageScore();
    return 0;
}

//解释addStudent：
//1. 首先检测学号是否重复
//2. 然后检测姓名是否为空
//3. 然后检测分数是否合法
//4. 然后向学生列表中添加学生
//5. 调用calculateAverageScore()函数，计算平均分
//6. 调用history.addAction()函数，记录操作

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

//解释deleteStudent：
//1. 首先遍历学生列表，找到要删除的学生的索引
//2. 然后调用history.addAction()函数，记录操作
//3. 然后删除学生
//4. 调用calculateAverageScore()函数，计算平均分

int Manager::modifyStudent(Student student, bool undoable)
{
    //首先检测姓名是否为空
    if (student.name.empty())
    {
        return 3; //姓名为空
    }
    for (int i = 0; i < courses.size(); i++)
    {
        if (student.scores[i] < -1.0f || (student.scores[i] > -1.0f && student.scores[i] < 0.0f))
        {
            return 2; //分数不合法
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

//解释modifyStudent：
//1. 首先检测姓名是否为空
//2. 然后检测分数是否合法
//3. 然后遍历学生列表，找到要修改的学生的索引
//4. 然后修改学生信息
//5. 调用calculateAverageScore()函数，计算平均分
//6. 调用history.addAction()函数，记录操作

void Manager::clearStudents()
{
    students.clear();
    calculateAverageScore();
    history.clear();
}

//解释clearStudents：
//1. 首先清空学生列表
//2. 然后调用calculateAverageScore()函数，计算平均分
//3. 调用history.clear()函数，清空操作记录

void Manager::insertRandom(int stu_count)
{
    newFile();
    courses.push_back(u8"语文");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"数学");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"英语");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"物理");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"化学");
    averageScores.push_back(-1.0f);
    courses.push_back(u8"生物");
    averageScores.push_back(-1.0f);

    random_device rd;   // 使用硬件随机数种子（如果可用）
    mt19937 gen(rd());  // Mersenne Twister 19937 引擎
    uniform_int_distribution<> distrib(-1, 100);  // 生成 1 到 100 之间的随机整数

    for (int i = 0; i < stu_count; i++) //添加stu_count个学生
    {
        vector<float> scores;
        for (int j = 0; j < manager.courses.size(); j++)
        {
            scores.push_back(distrib(gen));
        }
        Student student(i + 1, u8"学生" + to_string(i + 1), scores);
        students.push_back(student);
    }

    calculateAverageScore();
    table.filter.initFilter();
    table.refreshTable();
}