#pragma once
#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <string>
#include <vector>

#include "Levenshtein.h"

void SplitString(string input, vector<string> &output, char delimiter);
bool IsWordsEqual(string str1, string str2);

#endif // !UTILITY_H
