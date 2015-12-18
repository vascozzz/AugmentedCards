#pragma once

#include <opencv\cv.h>
#include <opencv2\core\core_c.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\nonfree\nonfree.hpp>
#include <opencv2\nonfree\features2d.hpp>

#include <iostream>
#include <vector>

#include "Rectangle.h"

using namespace std;
using namespace cv;

struct Card
{
	bool isNumber;
	string symbol;
	string suit;

	vector<Point> contours;
	Rectangle rectangle;

	Mat image, descriptors;
	vector<KeyPoint> keyPoints;
};