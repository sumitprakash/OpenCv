#include "Levenshtein.h"

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)));

int LevenshteinDistance(string &s1, string &s2)
{
	unsigned int s1Len, s2Len, x, y, lastDiag, oldDiag;
	s1Len = s1.size();
	s2Len = s2.size();
	unsigned int* column = new unsigned int[s1Len + 1];
	for (y = 1; y <= s1Len; ++y)
		column[y] = y;

	for (x = 1; x <= s2Len; ++x)
	{
		column[0] = x;
		for (y = 1, lastDiag = x - 1; y <= s1Len; ++y)
		{
			oldDiag = column[y];
			column[y] = MIN3(column[y] + 1, column[y - 1] + 1, lastDiag + (tolower(s1[y - 1]) == tolower(s2[x - 1]) ? 0 : 1));
			lastDiag = oldDiag;
		}
	}

	return(column[s1Len]);
}

bool StringCompare(string origStr, string &tkn, int maxDist)
{
	int dist = LevenshteinDistance(origStr, tkn);

	if (origStr.size() <= 4)
		maxDist = 1;

	if (dist <= maxDist)
		return true;
	else
		return false;
}