#ifndef OCR_H
#define OCR_H

#include	<fstream>

#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\ml.hpp>

#include "Preprocessing.h"

void Extract(Mat &grayScaleImg, Mat &thresholdImg, vector<ContourWithData> &allContours, int offsetX = 0, int offsetY = 0, bool recursive = false);
void ExtractContours(Mat &origImg, ContourWithData contour, vector<ContourWithData> &allContours);
void Predict(string filename, map<int, map<int, string>> &result, bool displayImage = true);
#endif // !OCR_H

