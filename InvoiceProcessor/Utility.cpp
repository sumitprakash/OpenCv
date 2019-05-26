#include "Utility.h"

void SplitString(string input, vector<string> &output, char delimiter)
{
	int pos = input.find(delimiter);
	if (pos != string::npos)
	{
		output.push_back(input.substr(0, pos));
		input = input.substr(pos + 1);
		return SplitString(input, output, delimiter);
	}
	else
	{
		output.push_back(input);
		return;
	}
}

bool IsWordsEqual(string str1, string str2)
{
	int str1Size = str1.size();
	int str2Size = str2.size();
	int differences = 0;
	int errThreshold = 0;

	if (str1Size == str2Size)
	{
		for (int i = 0; i < str1Size; i++)
		{
			if (tolower(str1.at(i)) != tolower(str2.at(i)))
				differences++;
		}

		if (str1Size <= 5)
			errThreshold = 1;
		else
			errThreshold = 2;

		if (differences <= errThreshold)
			return true;
		else
			return false;
	}
	else
		return false;
}