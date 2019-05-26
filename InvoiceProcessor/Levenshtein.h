#pragma once
#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#include <string>

using namespace std;

bool StringCompare(string originalstring, string &token, int maxDistance = 2);

#endif // !LEVENSHTEIN_H

