#pragma once
#include <string>
#include <vector>
using namespace std;
struct Student
{
public:
	long long id;
	string name;
	vector<float> scores; //score如果为-1 则表示该科目没有考试
	float totalScore;
	Student(int id, string name, vector<float> scores) : id(id), name(name), scores(scores)
	{
		totalScore = 0;
		for (int i = 0; i < scores.size(); i++)
		{
			if (scores[i] == -1)
			{
				continue;
			}
			totalScore += scores[i];
		}
	}
	void calculateTotalScore();

	string getName_str();
};
