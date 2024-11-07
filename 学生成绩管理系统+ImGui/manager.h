#pragma once
#include <Windows.h>
#include "student.h"
#include <map>

struct Manager {
	vector<Student> students;
	vector<float> averageScores;
	float totalAverageScore;
	vector<string> courses;
	Manager();
	string getCourseName_str(int index);
	void calculateAverageScore();
	int addStudent(Student student, bool undoable = true);
	void deleteStudent(int id, bool undoable = true);
	int modifyStudent(Student student, bool undoable = true);
	void clearStudents();
	int addCourse(string course, bool undoable = true);
	void addCourse(string course, vector<float> courseScores, bool undoable = true);
	void deleteCourse(string course, bool undoable = true);
	int modifyCourse(string course, string newCourse, bool undoable = true);
	void clearCourses();
	void insertRandom(int stu_count);
	void newFile();
	void save();
	void saveToFile();
	void saveToFileAs();
	int loadFromFile(bool firstLoad = false);
};

extern Manager manager;

