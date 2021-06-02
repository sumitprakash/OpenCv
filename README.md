# OpenCv
Project intends to showcase implementation of machine learning to exatract text from images.Here, OCR is developed using opencv in c++ and process the result to extract retrieve fields from invoice. Here, we have used SVM algorithm to predict text.
Note: This is only for reference and does not folow industry standards of coding.

# How To Use Program
This is console application which will run on windows operating system.
This application has two parts:
1.	Generate xml files for training
	1.	First place all the training images in the application folder.
	2.	Run the exe with command line argument “generate”. This will create xml files required for SVM model.
2.	Extract text from image
	1.	Run the exe with two command line arguments. First is “predict” and second is absolute file path of the image for text extraction.
	2.	All the result will be displayed on the console
