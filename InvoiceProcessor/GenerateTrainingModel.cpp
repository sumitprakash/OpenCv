#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<opencv2\ml\ml.hpp>

#include<iostream>
#include<conio.h>
#include<vector>
#include<string>
#include<fstream>

using namespace std;
using namespace cv;

bool FileExist(string filename)
{
	ifstream file(filename);
	if (!file)            
		return false;    
	else                
		return true;
}

void LoadFiles(Mat &classifications, Mat &trainingImg)
{
	FileStorage fsClassifications("classifications.xml", FileStorage::READ);
	if (fsClassifications.isOpened() == false)
	{
		cout << "error: unable to open training classification file, exiting program.";
		cv::waitKey(0);
		return;
	}
	else
	{
		fsClassifications["classifications"] >> classifications;
	}
	fsClassifications.release();

	FileStorage fsTrainingImg("images.xml", FileStorage::READ);
	if (fsTrainingImg.isOpened() == false)
	{
		cout << "error: unable to open training image file, exiting program.";
		cv::waitKey(0);
		return;
	}
	else
	{
		fsTrainingImg["images"] >> trainingImg;
	}
	fsTrainingImg.release();
}

void WriteToFiles(Mat &classifiactions, Mat &trainingImg)
{
	FileStorage fsClassifications("classifications.xml", FileStorage::WRITE);
	if (fsClassifications.isOpened() == false)
	{
		cout << "error: unable to open training classification file, exiting program.";
		cv::waitKey(0);
		return;
	}
	fsClassifications << "classifications" << classifiactions;
	fsClassifications.release();

	FileStorage fsTrainingImg("images.xml", FileStorage::WRITE);
	if (fsTrainingImg.isOpened() == false)
	{
		cout << "error: unable to open training image file, exiting program.";
		cv::waitKey(0);
		return;
	}
	fsTrainingImg << "images" << trainingImg;
	fsTrainingImg.release();
}

void CreateData(string file)
{
	Mat imgTrainingNumbers, imgGrayscale, imgBlurred, imgThresh;
	vector<vector<Point>> ptContours;
	vector<Vec4i> v4Heirarchy;

	vector<int> intValidChars = { '0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O' ,'P','Q','R' ,'S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
		'$','/','.',':' };

	Mat classifiactions;
	Mat trainingImg;

	LoadFiles(classifiactions, trainingImg);

	imgTrainingNumbers = imread(file);

	if (imgTrainingNumbers.empty())
	{
		cout << "error: Could not find file "<<file;
		cv::waitKey(0);
		return;
	}

	cvtColor(imgTrainingNumbers, imgGrayscale, COLOR_BGR2GRAY);

	GaussianBlur(imgGrayscale, imgBlurred, Size(3, 3), 0);

	adaptiveThreshold(imgGrayscale, imgThresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);

	findContours(imgThresh, ptContours, v4Heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	Mat roi;
	Mat roiResized;
	Rect boundingRect;
	int charInt;
	
	if (file == "forward_slash.png")
	{
		charInt = (int)('/');
	}
	else if (file == "dollar.png")
	{
		charInt = (int)('$');
	}
	else if(file == "dot.png")
	{
		charInt = (int)('.');
	}
	else if (file == "colon.png")
	{
		charInt = (int)(':');
	}
	else
	{
		charInt = (int)(file[0]);
	}
	for (int i = 0; i < ptContours.size(); i++)
	{
		boundingRect = cv::boundingRect(ptContours[i]);
		roi = imgThresh(boundingRect);
		resize(roi, roiResized, cv::Size(20, 30));
		if (std::find(intValidChars.begin(), intValidChars.end(), charInt) != intValidChars.end())
		{
			classifiactions.push_back(charInt);
			Mat ImgFloat;
			roiResized.convertTo(ImgFloat, CV_32FC1);
			Mat flatImg = ImgFloat.reshape(1, 1);
			trainingImg.push_back(flatImg);
		}
		else
			continue;
	}

	WriteToFiles(classifiactions, trainingImg);
}

void GenerateTrainingMoodel()
{
	vector<char> files = { '0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O' ,'P','Q','R' ,'S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
		'$','/','.',':' };

	string filename = "*.png";
	for (char file : files)
	{
		if (file == '/')
		{
			filename = "forward_slash.png";
		}
		else if (file == '$')
		{
			filename = "dollar.png";
		}
		else if (file == '.')
		{
			filename = "dot.png";
		}
		else if (file == ':')
		{
			filename = "colon.png";
		}
		else
		{
			filename[0] = file;
		}

		if (FileExist(filename))
			CreateData(filename);

	}
}