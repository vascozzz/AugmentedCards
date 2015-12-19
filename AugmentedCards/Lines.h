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

/*
 * Contains multiple structures for lines and operations with lines.
 */

struct Line
{
	int seq;
	Point2f p1, p2;
	float distance;
};

struct CompareLineDistance
{
	bool operator()(const Line& a, const Line& b)
	{
		if (a.distance < b.distance)
			return true;

		return false;
	}
};

struct CompareLineSequence
{
	bool operator()(const Line& a, const Line& b)
	{
		if (a.seq < b.seq)
			return true;

		return false;
	}
};