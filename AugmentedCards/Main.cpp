#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

bool compareContourArea(vector<Point>, vector<Point>);
void appendMats(Mat frame, Mat section, int x, int y);
float calculateDistance(Point2f p1, Point2f p2);
void preprocess(Mat &card);
void detectCards();

enum CardSuit
{
	HEARTS, DIAMONDS, CLUBS, SPADES, GRAYJOKER, COLORJOKER
};

struct Card
{
	int number;
	CardSuit suit;
};

Card deck[54];

int main(int argc, char** argv)
{
	//train();
	detectCards();
	waitKey(0);
}

void detectCards()
{
	const int nCards = 2;

	RNG rng(12345);
	Mat original, processing, contoursRender, rectsRender, perspectiveRender;
	Mat cardArray[nCards];
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;

	namedWindow("final", WINDOW_AUTOSIZE);
	namedWindow("diff", WINDOW_AUTOSIZE);

	// read from image
	original = imread("../Assets/cards.jpg", IMREAD_COLOR);

	if (original.empty())
	{
		cout << "Could not open or find the image" << endl;
		return;
	}

	// grayscale, blur, threshold
	cvtColor(original, processing, CV_BGR2GRAY);
	GaussianBlur(processing, processing, Size(1, 1), 1000);
	threshold(processing, processing, 120, 255, THRESH_BINARY);

	// edge detection and contours
	Canny(processing, processing, 0, 60, 3);
	findContours(processing, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// check for number of cards
	if ((int)contours.size() < nCards)
	{
		return;
	}

	// sorting by largest area
	sort(contours.begin(), contours.end(), compareContourArea);

	// prepare rendering frames
	contoursRender = Mat::zeros(processing.size(), CV_8UC3);
	rectsRender = Mat::zeros(processing.size(), CV_8UC3);

	// set rectangles points on a new perspective
	Point2f transfPoints[4];
	transfPoints[0] = Point2f(0, 449);
	transfPoints[1] = Point2f(0, 0);
	transfPoints[2] = Point2f(449, 0);
	transfPoints[3] = Point2f(449, 449);

	// process detected cards
	for (int i = 0; i < nCards; i++)
	{
		// card contours
		vector<Point> contour = contours[i];

		// rectangle from the contours, only 4 points
		RotatedRect rect = minAreaRect(contour);
		Point2f rectPoints[4];
		rect.points(rectPoints);

		if (calculateDistance(rectPoints[0], rectPoints[1]) < calculateDistance(rectPoints[1], rectPoints[2]))
		{
			Point2f p0 = rectPoints[0];
			rectPoints[0] = rectPoints[1];
			rectPoints[1] = rectPoints[2];
			rectPoints[2] = rectPoints[3];
			rectPoints[3] = p0;
		}

		// warp points in the original image according to the new perspective
		Mat transform = getPerspectiveTransform(rectPoints, transfPoints);
		warpPerspective(original, cardArray[i], transform, Size(450, 450));
		preprocess(cardArray[i]);

		// process rendering
		//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		//// render contours
		//drawContours(contoursRender, contours, i, color, 1, CV_AA, hierarchy, 0, Point());

		//// render rectangle
		//for (int j = 0; j < 4; j++)
		//{
		//	line(rectsRender, rectPoints[j], rectPoints[(j + 1) % 4], color, 1, 8);
		//}
	}

	//We now have each detected card processed. Now we compare them to our card database
	Mat cardDatabase = imread("../Assets/cardbase.png", IMREAD_GRAYSCALE);

	for (int i = 0; i < nCards; i++) //for each detected card
	{
		Mat detectedCard = cardArray[i];
		Mat flippedCard;
		flip(detectedCard, flippedCard, -1);

		Mat bestDiff;
		int bestValue = 9999999999999999;
		int bestIndex;

		for (int j = 0; j < 54; j++) //we compare every card in the database
		{
			Mat dbcard = cardDatabase(CvRect(j * 450, 0, 450, 450));
			
			Mat diff;
			Mat flippedDiff;

			absdiff(detectedCard, dbcard, diff);
			absdiff(flippedCard, dbcard, flippedDiff);

			GaussianBlur(diff, diff, Size(5, 5), 5);
			threshold(diff, diff, 200, 255, THRESH_BINARY);
			GaussianBlur(flippedDiff, flippedDiff, Size(5, 5), 5);
			threshold(flippedDiff, flippedDiff, 200, 255, THRESH_BINARY);

			int nonZero = countNonZero(diff);
			int nonZeroF = countNonZero(flippedDiff);

			if (nonZero < bestValue)
			{
				bestValue = nonZero;
				bestIndex = j;
				bestDiff = diff;
			}

			if (nonZeroF < bestValue)
			{
				bestValue = nonZeroF;
				bestIndex = j;
				bestDiff = flippedDiff;
			}

			//if (j == 44)
			//	imshow("diff", flippedCard);
		}

		Mat bestCard = cardDatabase(CvRect(bestIndex * 450, 0, 450, 450));
		imshow("final", bestCard);
		imshow("diff", bestDiff);
	}
}

int train()
{
	const int nCards = 54;

	RNG rng(12345);
	Mat original, processing, contoursRender, rectsRender, perspectiveRender;
	Mat cardArray[nCards];
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;

	//namedWindow("original", 1);
	//namedWindow("processing", 1);
	//namedWindow("contours", 1);
	//namedWindow("rectangles", 1);
	//namedWindow("perspective", 1);

	//for (int i = 0; i < nCards; i++)
	//{
	//	stringstream s;
	//	s << i;
	//	namedWindow("card" + s.str(), 1);
	//}

	namedWindow("final", 1);

	// read from image
	original = imread("../Assets/cards.png", IMREAD_COLOR);

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

	// set rectangles points on a new perspective
	Point2f transfPoints[4];
	transfPoints[0] = Point2f(0, 449);
	transfPoints[1] = Point2f(0, 0);
	transfPoints[2] = Point2f(449, 0);
	transfPoints[3] = Point2f(449, 449);

	// process detected cards
	for (int i = 0; i < nCards; i++)
	{
		// card contours
		vector<Point> contour = contours[i];

		// rectangle from the contours, only 4 points
		RotatedRect rect = minAreaRect(contour);
		Point2f rectPoints[4];
		rect.points(rectPoints);

		if (calculateDistance(rectPoints[0], rectPoints[1]) < calculateDistance(rectPoints[1], rectPoints[2]))
		{
			Point2f p0 = rectPoints[0];
			rectPoints[0] = rectPoints[1];
			rectPoints[1] = rectPoints[2];
			rectPoints[2] = rectPoints[3];
			rectPoints[3] = p0;
		}

		// warp points in the original image according to the new perspective
		Mat transform = getPerspectiveTransform(rectPoints, transfPoints);
		warpPerspective(original, cardArray[i], transform, Size(450, 450));

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

	//imshow("original", original);
	//imshow("processing", processing);
	//imshow("contours", contoursRender); 
	//imshow("rectangles", rectsRender);
	//imshow("perspective", perspectiveRender);

	Mat cardbase = Mat::zeros(Size(450 * nCards, 450), CV_8UC3);

	for (int i = 0; i < nCards; i++)
	{
		appendMats(cardbase, cardArray[i], 450 * i, 0);
	}


	//pre-process each card
	preprocess(cardbase);

	imshow("final", cardbase);
	imwrite("../Assets/cardbase.png", cardbase);

	waitKey(0);
	return 0;
}

bool compareContourArea(vector<Point> i, vector<Point> j)
{
	// second parameter controls the orientation
	// "true" avoids duplicates when sorting
	return contourArea(i, true) > contourArea(j, true);
}


void appendMats(Mat frame, Mat section, int x, int y)
{
	for (int i = 0; i < section.rows; i++)
	{
		for (int j = 0; j < section.cols; j++)
		{
			Vec3b pixel = section.at<Vec3b>(i, j);
			frame.at<Vec3b>(i + y, j + x) = pixel;
		}
	}
}

float calculateDistance(Point2f p1, Point2f p2)
{
	float diffY = p1.y - p2.y;
	float diffX = p1.x - p2.x;
	return sqrt((diffY * diffY) + (diffX * diffX));
}

void preprocess(Mat &image)
{
	cvtColor(image, image, CV_BGR2GRAY);
	GaussianBlur(image, image, Size(5, 5), 2);
	//threshold(image, image, 120, 255, THRESH_BINARY);
	adaptiveThreshold(image, image, 255, 1, 1, 11, 1);
}