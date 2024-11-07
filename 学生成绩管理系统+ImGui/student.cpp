#include "student.h"
#include "utf8_to_string.h"
string Student::getName_str()
{
	return utf8_to_string(name);
}

void Student::calculateTotalScore()
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