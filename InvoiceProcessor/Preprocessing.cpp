#include <iostream>
#include <cmath>
#include <opencv2\opencv.hpp>
#include "Preprocessing.h"

bool ContourWithData::IsValidContour()
{
	if (boundingRect.width*boundingRect.height <= 2 || boundingRect.height > 35 || boundingRect.width > 25)
	{
		return false;
	}
	return true;
}

bool CheckValidContour(int width, int height)
{
	if (width * height < MIN_CONTOUR_AREA)
	{
		return false;
	}
	return true;
}

int GetHorizontalSpace(int height)
{
	if (height <= 11)
		return 5;
	else if (height >= 12 && height <= 15)
		return 5;
	else if (height >= 21 && height <= 25)
		return 8;
	else
		return 8;
}

void RemoveTable(Mat &bwImage)
{
	vector<vector<Point>> ptContourHor;
	vector<Vec4i> v4iHierarchyHor;
	vector<vector<Point>> ptContourVer;
	vector<Vec4i> v4iHierarchyVer;
	vector<Rect> horRect;
	vector<Rect> verRect;

	Mat horizontal = bwImage.clone();
	Mat vertical = bwImage.clone();

	int horizontalsize = 40;

	Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontalsize, 1));

	erode(horizontal, horizontal, horizontalStructure, Point(-1, -1));
	dilate(horizontal, horizontal, horizontalStructure, Point(-1, -1));

	int verticalsize = (horizontal.rows / 10) < 30 ? (horizontal.rows / 10) : 30;
	Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(1, verticalsize));

	erode(vertical, vertical, verticalStructure, Point(-1, -1));
	dilate(vertical, vertical, verticalStructure, Point(-1, -1));

	findContours(horizontal, ptContourHor, v4iHierarchyHor, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	findContours(vertical, ptContourVer, v4iHierarchyVer, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	for (int i = 0; i < ptContourHor.size(); i++)
	{
		horRect.push_back(boundingRect(ptContourHor[i]));
	}
	for (int i = 0; i < ptContourVer.size(); i++)
	{
		Rect temp = boundingRect(ptContourVer[i]);
		if (temp.height > 40)
		{
			verRect.push_back(temp);
		}
		else
		{
			for (int i = 0; i < horRect.size(); i++)
			{
				if (temp.y >= horRect[i].y - 4 && temp.y <= horRect[i].y + 4)
				{
					if (temp.x >= horRect[i].x - 4 && temp.x <= horRect[i].x + horRect[i].width + 4)
					{
						verRect.push_back(temp);
						break;
					}
				}
			}
		}
	}

	int max = horRect.size();
	if (verRect.size() > horRect.size())
	{
		max = verRect.size();
	}
	for (int i = 0; i < max; i++)
	{
		if (i < verRect.size())
		{
			if (i < verRect.size())
			{
				if (verRect[i].x > 0)
				{
					verRect[i].x--;
					verRect[i].width++;
				}
			}
			if (verRect[i].y > 0)
			{
				verRect[i].y--;
				verRect[i].height++;
			}
			bwImage(verRect[i]) = 0;
		}
		if (i < horRect.size())
		{
			if (horRect[i].x > 0)
			{
				horRect[i].x--;
				horRect[i].width++;
			}
			if (horRect[i].y > 0)
			{
				horRect[i].y--;
				horRect[i].height++;
			}
			bwImage(horRect[i]) = 0;
		}
	}
}

void SigmoidThreshold(Mat &Image, int threshold)
{
	int rows = Image.rows;
	int cols = Image.cols;
	for (int x = 0; x < rows; x++)
	{
		for (int y = 0; y < cols; y++)
		{
			if (Image.at<uchar>(x, y) < (uchar)threshold)
				Image.at<uchar>(x, y) < (uchar)0;
			else
				Image.at<uchar>(x, y) < (uchar)255;
		}
	}
}

void SplitContour(Mat &originalImage, ContourWithData contour, vector<ContourWithData> &allContours)
{
	Mat image = originalImage(contour.boundingRect);
	int cols = image.cols;
	int rows = image.rows;
	int mid = cols / 2;
	int zeroCount = 0;
	int maxZero = 0;
	int pos = mid;
	map<int, int> minPos;

	if (mid < 4)
	{
		allContours.push_back(contour);
		return;
	}

	for (int y = mid - 2; y <= mid + 2; y++)
	{
		for (int x = 0; x < rows; x++)
		{
			if ((int)image.at<uchar>(x, y) == 0)
				zeroCount++;
		}

		if (zeroCount >= maxZero)
		{
			maxZero = zeroCount;
			pos = y;
			minPos[y] = zeroCount;
		}
		zeroCount = 0;
	}

	auto firstItr = minPos.begin();
	auto secondItr = minPos.begin();
	if (secondItr != minPos.end())
		secondItr;

	int ypos;
	for (;;)
	{
		if (secondItr != minPos.end())
			break;
		if (firstItr != minPos.end())
			break;
		if (secondItr->second > firstItr->second)
		{
			ypos = firstItr->first;
		}
		else if (secondItr->second == firstItr->second)
		{
			if (abs(mid - firstItr->first) >= abs(mid - secondItr->first))
				ypos = firstItr->first;
			else
				ypos = secondItr->first;
		}
		else
			ypos = secondItr->first;

		secondItr++;
		firstItr++;
		minPos.erase(ypos);
	}
	if (minPos.size() == 1)
		pos = minPos.begin()->first;

	ContourWithData tempContour;

	tempContour.boundingRect.y = contour.boundingRect.y;
	tempContour.boundingRect.x = contour.boundingRect.x;
	tempContour.boundingRect.width = pos + 1;
	tempContour.boundingRect.height = contour.boundingRect.height;
	allContours.push_back(tempContour);

	tempContour.boundingRect.y = contour.boundingRect.y;
	tempContour.boundingRect.x = contour.boundingRect.x + pos + 1;
	tempContour.boundingRect.width = contour.boundingRect.width - pos + 1;
	tempContour.boundingRect.height = contour.boundingRect.height;
	allContours.push_back(tempContour);
}

void ContourSort(vector<ContourWithData> validContourData, map<int, map<int, ContourWithData>> &sortedContour)
{
	int size = validContourData.size();
	Rect boundingRect;
	for (int i = 0; i < size; i++)
	{
		boundingRect = validContourData[i].boundingRect;
		sortedContour[boundingRect.y][boundingRect.x] = validContourData[i];
	}
}

void Validate(map<int, map<int, ContourWithData>> &sortedContour)
{
	map<int, map<int, ContourWithData>>::iterator outerItr = sortedContour.begin();
	map<int, ContourWithData>::iterator innerItr;

	for (; outerItr != sortedContour.end();)
	{
		innerItr = outerItr->second.begin();
		for (; innerItr != outerItr->second.end();)
		{
			bool found = false;
			if (!CheckValidContour(innerItr->second.boundingRect.width, innerItr->second.boundingRect.height))
			{
				auto tempOuterItr = outerItr;
				if (outerItr != sortedContour.end())
				{
					tempOuterItr++;
				}
				else
				{
					break;
				}

				for (; tempOuterItr != sortedContour.end() && tempOuterItr->first - innerItr->second.boundingRect.y - innerItr->second.boundingRect.height <= 5; tempOuterItr++)
				{
					int y = tempOuterItr->first;
					auto tempInnerItr = tempOuterItr->second.begin();
					for (; tempInnerItr != tempOuterItr->second.end() && tempInnerItr->first <= innerItr->first + 1; tempInnerItr++)
					{
						int x = tempInnerItr->first;
						if (sortedContour.find(y) != sortedContour.end())
						{
							if (sortedContour[y].find(x) != sortedContour[y].end())
							{
								if (innerItr->first + 1 >= sortedContour[y][x].boundingRect.x&&innerItr->first <= sortedContour[y][x].boundingRect.x + sortedContour[y][x].boundingRect.width)
								{
									innerItr->second.boundingRect.width = innerItr->second.boundingRect.width + innerItr->second.boundingRect.x - x;
									innerItr->second.boundingRect.height = sortedContour[y][x].boundingRect.height + y - innerItr->second.boundingRect.y;
									innerItr->second.boundingRect.x = x;
									sortedContour[y].erase(x);
									if (sortedContour[y].begin() == sortedContour[y].end())
									{
										sortedContour.erase(y);
									}
									found = true;
								}
							}
						}
						if (found)
							break;
					}
					if (found)
						break;
				}
			}
			innerItr++;
		}
		outerItr++;
	}
}

void ContourMerge(map<int, map<int, ContourWithData>> &sortedContour)
{
	int ypos;
	int maxHeight = 0;
	map<int, map<int, ContourWithData>>::iterator secItr = sortedContour.begin();
	map<int, map<int, ContourWithData>>::iterator firsItr = sortedContour.begin();

	try
	{
		for (; firsItr != sortedContour.end(); firsItr++)
		{
			maxHeight = 0;
			try
			{
				for (auto tempItr : firsItr->second)
				{
					if (tempItr.second.boundingRect.height > maxHeight)
						maxHeight = tempItr.second.boundingRect.height;
				}
			}
			catch (const std::exception ex)
			{
				cout << ex.what();
			}

			auto temp = firsItr;
			try
			{
				for (secItr = ++temp; secItr != sortedContour.end();)
				{
					if (firsItr->first + maxHeight - secItr->first > 0)
					{
						for (auto tempItr : secItr->second)
						{
							if (maxHeight < tempItr.second.boundingRect.height)
							{
								maxHeight = tempItr.second.boundingRect.height;
							}
							firsItr->second[tempItr.first] = tempItr.second;
						}
						ypos = secItr->first;
						secItr++;
						sortedContour.erase(ypos);
					}
					else
						break;
				}
			}
			catch (const std::exception ex)
			{
				cout << ex.what();
			}

			map<int, map<int, ContourWithData>>::iterator outerItr = sortedContour.begin();
			map<int, ContourWithData>::iterator innerItr1;
			map<int, ContourWithData>::iterator innerItr2;
			int xpos;
			int i = 0;
			try
			{
				for (; outerItr != sortedContour.end();)
				{
					innerItr1 = outerItr->second.begin();
					innerItr2 = outerItr->second.begin();
					innerItr2++;

					for (; innerItr2 != outerItr->second.end();)
					{
						++i;
						//if (i == 231)
						//	cout << "here" << endl;
						//if (i == 232)
						//	cout << "here" << endl;

						if (outerItr->second.end() != innerItr1 && ((innerItr1->second.boundingRect.x + innerItr1->second.boundingRect.width - 1) > innerItr2->second.boundingRect.x))
						{
							if (innerItr2->second.boundingRect.x + innerItr2->second.boundingRect.width - innerItr1->second.boundingRect.x - ((innerItr1->second.boundingRect.height > innerItr2->second.boundingRect.height) ? innerItr1->second.boundingRect.height : innerItr2->second.boundingRect.height) <= 5)
							{
								innerItr1->second.boundingRect.width = innerItr2->second.boundingRect.x + innerItr2->second.boundingRect.width - innerItr1->second.boundingRect.x;
								if (innerItr1->second.boundingRect.y > innerItr2->second.boundingRect.y)
								{
									innerItr1->second.boundingRect.y = innerItr2->second.boundingRect.y;
								}
								if (innerItr1->second.boundingRect.height < innerItr2->second.boundingRect.height)
								{
									innerItr1->second.boundingRect.height = innerItr2->second.boundingRect.height;
								}
								xpos = innerItr2->second.boundingRect.x;
								innerItr2++;
								if (innerItr1->first == xpos)
									innerItr1 = outerItr->second.end();
								sortedContour[outerItr->first].erase(xpos);
							}
							else
							{
								innerItr1++;
								innerItr2++;
							}
						}
						else
						{
							innerItr1++;
							innerItr2++;
						}
					}
					outerItr++;
				}
			}
			catch (const std::exception ex)
			{
				cout << ex.what();
			}
		}
	}
	catch (const std::exception& ex)
	{
		cout << ex.what();
	}
}

void TrimImage(Mat &originalImage)
{
	int cols = originalImage.cols;
	int rows = originalImage.rows;
	int maxPoints = 5;
	int pointsCount = 0;
	int y1 = -1, y2 = -1, x1 = -1, x2 = -1;
	for (int row = 0; row < rows; row++)
	{
		pointsCount = 0;
		for (int col = 0; col < cols; col++)
		{
			if (originalImage.at<uchar>(row, col) == (uchar)255)
				pointsCount++;
		}
		if (pointsCount <= maxPoints)
		{
			y1 = row;
		}
		else
			break;
	}
	y1--;
	for (int row = rows - 1; rows >= 0; row--)
	{
		pointsCount = 0;
		for (int col = 0; col < cols; col++)
		{
			if (originalImage.at<uchar>(row, col) == (uchar)255)
				pointsCount++;
		}
		if (pointsCount <= maxPoints)
		{
			y2 = row;
		}
		else
			break;
	}
	y2++;

	for (int col = 0; col < cols; col++)
	{
		pointsCount = 0;
		for (int row = 0; row < rows; row++)
		{
			if (originalImage.at<uchar>(row, col) == (uchar)255)
				pointsCount++;
		}
		if (pointsCount <= maxPoints)
			x1 = col;
		else
			break;
	}
	x1--;

	for (int col = cols - 1; col >= 0; col--)
	{
		pointsCount = 0;
		for (int row = 0; row < rows; rows++)
		{
			if (originalImage.at<uchar>(row, col) == (uchar)255)
				pointsCount++;
			if (pointsCount <= maxPoints)
				x2 = cols;
			else
				break;
		}
	}
	x2++;

	int x = x1, y = y1;
	if (x1 <= -1)
		x1 = 0;
	if (x2 <= -1)
		x2 = cols + 1;
	if (y1 <= -1)
		y1 = 0;
	if (y2 <= -1)
		y2 = cols + 1;

	originalImage = originalImage(Rect(x + 1, y + 1, x2 - x1, y2 - y1));
}