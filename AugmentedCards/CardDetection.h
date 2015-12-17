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

	Mat image, descriptors;
	vector<KeyPoint> keyPoints;
};

struct Rectangle
{
	Point p1, p2, p3, p4;
};

void train(string filename, int nCards, DetectionMethod method);
vector<Card> readDeckList(string filename);
void readDeckImage(string filename, vector<Card> &deck, DetectionMethod method);

bool isNumber(string number);
bool compareContourArea(vector<Point> v1, vector<Point> v2);
void appendToMat(Mat image, Mat section, int x, int y);
float calculateDistance(Point2f p1, Point2f p2);

void preprocess(Mat &image);
vector<vector<Point>> getContours(Mat image, int nCards);
Rectangle getCardRectangle(vector<Point> contour);

Mat getCardPerspective(Mat image, Rectangle rectangle, DetectionMethod method);
Card detectCard(Mat perspective, vector<Card> deck, DetectionMethod method);

int detectCardBinary(Mat card, Mat flipped, vector<Card> deck);
int getBinaryDiff(Mat detectedCard, Mat deckCard);

int detectCardSurf(Mat card, Mat flipped, vector<Card> deck);
int getSurfMatches(vector<KeyPoint> keyPoints1, Mat descriptors1, vector<KeyPoint> keyPoints2, Mat descriptors2);
void filterMatchesByAbsoluteValue(std::vector<DMatch> &matches, float maxDistance);
Mat filterMatchesRANSAC(vector<DMatch> &matches, vector<KeyPoint> &keypointsA, vector<KeyPoint> &keypointsB);
