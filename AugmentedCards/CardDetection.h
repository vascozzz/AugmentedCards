#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>

using namespace cv;
using namespace std;

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
bool compareContourArea(vector<Point> i, vector<Point> j);
void appendToMat(Mat image, Mat section, int x, int y);
float calculateDistance(Point2f p1, Point2f p2);

void preprocess(Mat &image);
int getCardDiff(Mat detectedCard, Mat deckCard);
vector<vector<Point>> getContours(Mat image, int nCards);
Rectangle getCardRectangle(vector<Point> contour);
Mat getCardPerspective(Mat image, Rectangle rectangle);
Card detectCard(Mat perspective, vector<Card> deck, Mat deckImage);