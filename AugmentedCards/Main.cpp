#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool compareContourArea(vector<Point>, vector<Point>);

int main(int argc, char** argv)
{
	RNG rng(12345);
	Mat original, processing, contoursRender, rectsRender, perspectiveRender;
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	int nCards = 2;

	namedWindow("original", 1);
	namedWindow("processing", 1);
	namedWindow("contours", 1);
	namedWindow("rectangles", 1);
	namedWindow("perspective", 1);

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

	// prepare rendering frames
	contoursRender = Mat::zeros(processing.size(), CV_8UC3);
	rectsRender = Mat::zeros(processing.size(), CV_8UC3);

	// process detected cards
	for (int i = 0; i < nCards; i++)
	{
		// card contours
		vector<Point> contour = contours[i];

		// rectangle from the contours, only 4 points
		RotatedRect rect = minAreaRect(contour);
		Point2f rectPoints[4];
		rect.points(rectPoints);

		// set rectangles points on a new perspective
		Point2f transfPoints[4];
		transfPoints[0] = Point2f(0, 449);
		transfPoints[1] = Point2f(0, 0);
		transfPoints[2] = Point2f(449, 0);
		transfPoints[3] = Point2f(449, 449);

		// warp points in the original image according to the new perspective
		Mat transform = getPerspectiveTransform(rectPoints, transfPoints);
		warpPerspective(original, perspectiveRender, transform, Size(450, 450));

		// process rendering
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		// render contours
		drawContours(contoursRender, contours, i, color, 1, CV_AA, hierarchy, 0, Point());

		// render rectangle
		for (int j = 0; j < 4; j++)
		{
			line(rectsRender, rectPoints[j], rectPoints[(j + 1) % 4], color, 1, 8);
		}
	}

	imshow("original", original);
	imshow("processing", processing);
	imshow("contours", contoursRender); 
	imshow("rectangles", rectsRender);
	imshow("perspective", perspectiveRender);

	waitKey(0);
	return 0;
}

bool compareContourArea(vector<Point> i, vector<Point> j)
{
	// second parameter controls the orientation
	// "true" avoids duplicates when sorting
	return contourArea(i, true) > contourArea(j, true);
}
