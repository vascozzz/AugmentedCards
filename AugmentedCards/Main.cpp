#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool compareContourArea(vector<Point> i, vector<Point> j);

int main(int argc, char** argv)
{
	RNG rng(12345);
	Mat original, processing, render;
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	int nCards = 2;

	namedWindow("original", 1);
	namedWindow("processing", 1);
	namedWindow("render", 1);

	// read from image
	original = imread("../Assets/cards.jpg", IMREAD_COLOR);

		if (original.empty())
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}

	// grayscale, blur, threshold
	cvtColor(original, processing, CV_BGR2GRAY);
	GaussianBlur(processing, processing, Size(1, 1), 1000);
	threshold(processing, processing, 120, 255, THRESH_BINARY);

	// edge detection and contours
	Canny(processing, processing, 0, 60, 3);
	findContours(processing, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// check for number of cards
	if ((int) contours.size() < nCards)
	{
		return 0;
	}

	// sorting by largest area
	sort(contours.begin(), contours.end(), compareContourArea);

	// draw detected cards
	render = Mat::zeros(processing.size(), CV_8UC3);

	for (int i = 0; i < nCards; i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(render, contours, i, color, 1, CV_AA, hierarchy, 0, Point());

		for (unsigned int x = 0; x < contours[i].size(); x++)
		{
			// cout << "Point " << x << " - x: " << contours[i][x].x << " | y: " << contours[i][x].y << endl;
		}

		cout << endl << endl << endl;
	}

	imshow("original", original);
	imshow("processing", processing);
	imshow("render", render);

	waitKey(0);
	return 0;
}

bool compareContourArea(vector<Point> i, vector<Point> j)
{
	// second parameter controls the orientation
	// "true" avoids duplicates when sorting
	return contourArea(i, true) > contourArea(j, true);
}
