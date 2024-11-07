#pragma once
#include <string>
#include <map>
using namespace std;
class Subject
{
public:
	map<int, string> subject_map;
	Subject(map<int, string> subject_map) : subject_map(subject_map) {}
};
