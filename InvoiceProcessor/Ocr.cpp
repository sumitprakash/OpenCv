#include "Ocr.h"
#include <conio.h>
using namespace cv::ml;

int MIN_CONTOUR_AREA = 64;
int RESIZED_IMG_WIDTH = 20;
int RESIZED_IMG_HEIGHT = 30;

int K = 1;
float C = 1;

void LoadDataFiles(Mat &trainingImgAsFloats, Mat &classificationInt)
{
	FileStorage classificationFile("classifications.xml", FileStorage::READ);
	if (classificationFile.isOpened() == false)
	{
		cout << "error: unable to open training classification file, exiting program.";
		cv::waitKey(0);
		exit(EXIT_FAILURE);
	}
	else
	{
		classificationFile["classifications"] >> classificationInt;
	}
	classificationFile.release();


	FileStorage trainingImgFile("images.xml", FileStorage::READ);
	if (trainingImgFile.isOpened() == false)
	{
		cout << "error: unable to open training image file, exiting program.";
		cv::waitKey(0);
		exit(EXIT_FAILURE);
	}
	else
	{
		trainingImgFile["images"] >> trainingImgAsFloats;
	}
	trainingImgFile.release();
}

void Padding(Mat &origImg, Mat &paddedImg)
{
	paddedImg.create(RESIZED_IMG_HEIGHT, RESIZED_IMG_WIDTH, origImg.type());
	paddedImg.setTo(Scalar::all(0));
	origImg.copyTo(paddedImg(Rect(0, 0, origImg.cols, origImg.rows)));
}

void ReprocessContour(Mat &origImg, ContourWithData contour, vector<ContourWithData> &allContours)
{
	Mat img = origImg(contour.boundingRect);
	Mat thresholdImg;
	Mat paddedImg;
	ContourWithData contourData;
	vector<vector<Point>> ptContours;
	vector<Vec4i> v4iHierarchy;
	
	int cols = img.cols;
	int rows = img.rows;

	struct points
	{
		int row;
		int col;
		bool flag;
	}pt;

	map<int, vector<points>> ptCollection;
	Mat matThreshold;
	int color;
	int prevColor, nextColor;
	for (int row = 0; row < rows; row++)
	{
		for (int col = 3; col < cols - 3; col++)
		{
			color = (int)img.at<uchar>(row, col - 1);
			if (color <= 60 || color >= 180)
			{
				continue;
			}
			pt.row = row;
			pt.col = col;
			pt.flag = false;
			prevColor = (int)img.at<uchar>(row, col - 1);
			nextColor = (int)img.at<uchar>(row, col + 1);
			if ((color > prevColor) && (color > nextColor) || (color < prevColor) && (color < nextColor))
			{
				pt.flag = true;
			}

			ptCollection[color].push_back(pt);
		}
	}

	for (auto r : ptCollection)
	{
		if (r.second.size() <= 2)
		{
			for (auto v : r.second)
			{
				if (v.flag)
				{
					if ((int)img.at<uchar>(v.row, v.col) > 127)
					{
						img.at<uchar>(v.row, v.col) = (uchar)0;
					}
					else
					{
						img.at<uchar>(v.row, v.col) = (uchar)255;
					}
				}
			}
		}
	}

	adaptiveThreshold(origImg, thresholdImg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 1);
	adaptiveThreshold(img, matThreshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 1);

	paddedImg.create(matThreshold.rows + 2, matThreshold.cols + 2, matThreshold.type());
	paddedImg.setTo(Scalar::all(0));
	matThreshold.copyTo(paddedImg(Rect(1, 1, matThreshold.cols, matThreshold.rows)));

	findContours(paddedImg, ptContours, v4iHierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	for (int i = 0; i < ptContours.size(); i++)
	{
		contourData.ptContour = ptContours[i];
		contourData.boundingRect = boundingRect(contourData.ptContour);
		contourData.fltArea = contourArea(contourData.ptContour);

		contourData.boundingRect.y = contourData.boundingRect.y - 1 + contour.boundingRect.y;
		contourData.boundingRect.x = contourData.boundingRect.x - 1 + contour.boundingRect.x;

		if (contourData.boundingRect.width - contourData.boundingRect.height > 4)
		{
			SplitContour(thresholdImg, contourData, allContours);
		}
		else
		{
			allContours.push_back(contourData);
		}
	}

}

void ExtractContours(Mat &grayscaleImg, Mat &thresholdImg, vector<ContourWithData> &allContours, int offsetX, int offsetY, bool recursive)
{
	vector<vector<Point>> ptContours;
	vector<Vec4i> v4iHierarchy;

	adaptiveThreshold(grayscaleImg, thresholdImg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 1);

	RemoveTable(thresholdImg);

	Mat thresholdImgCopy = thresholdImg.clone();

	findContours(thresholdImgCopy, ptContours, v4iHierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	ContourWithData contourData;

	for (int i = 0; i < ptContours.size(); i++)
	{
		contourData.ptContour = ptContours[i];
		contourData.boundingRect = boundingRect(contourData.ptContour);
		contourData.fltArea = contourArea(contourData.ptContour);
		contourData.boundingRect.y = contourData.boundingRect.y + offsetY;
		contourData.boundingRect.x = contourData.boundingRect.x + offsetX;
		Mat t = thresholdImg(contourData.boundingRect);
		if (contourData.boundingRect.width - contourData.boundingRect.height > 4)
		{
			SplitContour(thresholdImg, contourData, allContours);
		}
		else
		{
			allContours.push_back(contourData);
		}
	}
}

bool TrainSVM(Mat &trainingImgAsFloats, Mat &classificationInt, Ptr<SVM> &svm)
{
	svm = SVM::create();
	svm->setType(SVM::C_SVC);
	svm->setKernel(SVM::LINEAR);
	svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));
	svm->train(classificationInt, ROW_SAMPLE, trainingImgAsFloats);
	//svm->trainAuto(classificationInt, ROW_SAMPLE, trainingImgAsFloats);
	return true;
}

void Predict(string imgFilename, map<int, map<int, string>> &result, bool displayImg)
{
	vector<ContourWithData> allContours;
	vector<ContourWithData> validContours;

	Mat classificationInt;
	Mat trainingImgFloat;

	Ptr<SVM> svm;
	Mat grayscale;
	Mat blurred;
	Mat threshold;
	Mat thresholdCopy;
	vector<vector<Point>> ptContours;
	vector<Vec4i> v4iHeirarchy;
	map<int, map<int, ContourWithData>> sortedContour;
	int nextPos = 0;
	int xPos = 0;
	Mat paddedImg;
	int width;
	int getMaxHorSpace = 4;
	Mat roi;
	Mat roiResized;
	Mat roiFloat;
	Mat roiFlattenedFloat;

	LoadDataFiles(classificationInt, trainingImgFloat);

	TrainSVM(trainingImgFloat, classificationInt, svm);
	
	Mat testingNumbers = imread(imgFilename);

	namedWindow("Image_Original", WINDOW_NORMAL);
	cv::imshow("Image_Original", testingNumbers);
	cv::waitKey(0);

	if (testingNumbers.empty())
	{
		cout << "error: could not read image file : "<<imgFilename;
		cv::waitKey(0);
		exit(EXIT_FAILURE);
	}

	GaussianBlur(testingNumbers, testingNumbers, Size(3, 3), 0, 0, BORDER_DEFAULT);

	cvtColor(testingNumbers, grayscale, COLOR_BGR2GRAY);

	try
	{
		ExtractContours(grayscale, threshold, allContours, 0, 0, true);

		for (int i = 0; i < allContours.size(); i++)
		{
			if (allContours[i].IsValidContour())
			{
				validContours.push_back(allContours[i]);
			}
		}
	}
	catch (exception ex)
	{
		cout << ex.what();
		cv::waitKey(0);
	}

	try
	{
		ContourSort(validContours, sortedContour);
		Validate(sortedContour);
		ContourMerge(sortedContour);
	}
	catch (const std::exception ex)
	{
		cout << ex.what();
		_getch();
	}

	for (auto temp1 : sortedContour)
	{
		getMaxHorSpace = 4;
		for (auto temp2 : temp1.second)
		{
			rectangle(testingNumbers, temp2.second.boundingRect, Scalar(0, 0, 255), 1);
			roi = threshold(temp2.second.boundingRect);
			width = temp2.second.boundingRect.width * RESIZED_IMG_HEIGHT / temp2.second.boundingRect.height;
			if (width > RESIZED_IMG_WIDTH)
			{
				width = RESIZED_IMG_WIDTH;
			}

			resize(roi, roiResized, cv::Size(RESIZED_IMG_WIDTH, RESIZED_IMG_HEIGHT));

			/*Padding(roiResized, paddedImg);
			dilate(paddedImg, paddedImg, getStructuringElement(MORPH_RECT, Size(2, 2)), Point(-1, -1));
			SigmoidThreshold(paddedImg, 170);*/
			paddedImg = roiResized;
			paddedImg.convertTo(roiFloat, CV_32FC1);
			roiFlattenedFloat = roiFloat.reshape(1, 1);

			float currentChar = svm->predict(roiFlattenedFloat);

			if (currentChar == (int)(':'))
			{
				continue;
			}
			if (currentChar == (int)('.'))
			{
				if (temp2.second.boundingRect.height / temp2.second.boundingRect.width >= 2)
				{
					currentChar = (int)'I';
				}
				else if (temp2.second.boundingRect.width / temp2.second.boundingRect.height >= 2)
				{
					currentChar = (int)'-';
				}
			}
			if (nextPos == 0 || (temp2.first - nextPos > getMaxHorSpace))
			{
				result[temp1.first][temp2.first] = char(int(currentChar));
				xPos = temp2.first;
			}
			else
			{
				result[temp1.first][xPos] += char(int(currentChar));
			}
			nextPos = temp2.first + temp2.second.boundingRect.width;
			getMaxHorSpace = GetHorizontalSpace(temp2.second.boundingRect.height) - 1;
		}
		nextPos = 0;
	}
	if (displayImg)
	{
		//namedWindow("Image_Modified", WINDOW_GUI_EXPANDED);
		//cv::imshow("Image_Modified", testingNumbers);
		//namedWindow("Image_Modified1", WINDOW_GUI_NORMAL);
		//cv::imshow("Image_Modified1", testingNumbers);
		namedWindow("Image_Modified", WINDOW_NORMAL);
		cv::imshow("Image_Modified", testingNumbers);
		cv::waitKey(0);
	}
}
