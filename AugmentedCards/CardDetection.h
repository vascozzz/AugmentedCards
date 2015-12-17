#pragma once

#include <opencv\cv.h>
#include <opencv2\core\core_c.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\nonfree\nonfree.hpp>
#include <opencv2\nonfree\features2d.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <limits>

using namespace cv;
using namespace std;

enum DetectionMethod
{
	Binary,
	Surf
};

struct Card
{
	bool isNumber;
	string symbol;
	string suit;
};

struct Rectangle
{
	Point p1, p2, p3, p4;
};

vector<Card> readDeck(string filename);
bool isNumber(string number);
bool compareContourArea(vector<Point> v1, vector<Point> v2);
void appendToMat(Mat image, Mat section, int x, int y);
float calculateDistance(Point2f p1, Point2f p2);
void preprocess(Mat &image);
vector<vector<Point>> getContours(Mat image, int nCards);
Rectangle getCardRectangle(vector<Point> contour);
Mat getCardPerspective(Mat image, Rectangle rectangle, DetectionMethod method);
int getBinaryDiff(Mat detectedCard, Mat deckCard);
int getSurfMatches(Mat image1, Mat image2);
void filterMatchesByAbsoluteValue(std::vector<DMatch> &matches, float maxDistance);
Mat filterMatchesRANSAC(vector<DMatch> &matches, vector<KeyPoint> &keypointsA, vector<KeyPoint> &keypointsB);
Card detectCard(Mat perspective, vector<Card> deck, Mat deckImage, DetectionMethod method);
void train(string filename, int nCards, DetectionMethod method);