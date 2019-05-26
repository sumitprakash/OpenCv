#pragma once
#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <iostream>
#include <opencv2\opencv.hpp>

using namespace cv;
using namespace std;

extern int MIN_CONTOUR_AREA;
extern int RESIZED_IMG_WIDTH;
extern int RESIZED_IMG_HEIGHT;

extern int K;
extern float C;

class ContourWithData
{
public:
	vector<Point> ptContour;
	Rect boundingRect;
	float fltArea;

	bool IsValidContour();
};

bool CheckValidContour(int width, int height);
int GetHorizontalSpace(int height);
void RemoveTable(Mat &bwImg);
void SigmoidThreshold(Mat &img, int threshold);
void SplitContour(Mat &originalImg, ContourWithData contour, vector<ContourWithData> &allContours);
void ContourSort(vector<ContourWithData> validContours, map<int, map<int, ContourWithData>> &sortedContours);
void Validate(map<int, map<int, ContourWithData>> &sortedContours);
void ContourMerge(map<int, map<int, ContourWithData>> &sortedContour);
void TrimImage(Mat &originalImg);

#endif // !PREPROCESSING_H
