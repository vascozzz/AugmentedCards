#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	VideoCapture cap(0);

	if (!cap.isOpened())
	{
		return -1;
	}

	Mat original;
	Mat edges;

	namedWindow("original", 1);
	namedWindow("edges", 1);

	while (cap.isOpened())
	{
		cap >> original;
		cvtColor(original, edges, CV_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);

		imshow("edges", edges);
		imshow("original", original);

		if (waitKey(30) >= 0) {
			break;
		}
	}

	return 0;
}