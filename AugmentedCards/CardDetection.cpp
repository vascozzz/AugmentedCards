#include "CardDetection.h"

vector<Card> readDeck(string filename)
{
	ifstream file(filename);
	stringstream stream;
	string line, word;

	vector<Card> deck;
	Card card;

	if (!file.is_open())
	{
		return deck;
	}

	while (getline(file, line))
	{
		stream = stringstream(line);
		stream >> word;

		// numbers / symbols
		card.symbol = word;

		if (isNumber(word))
		{
			card.isNumber = true;
		}
		else
		{
			card.isNumber = false;
		}

		// suits
		stream >> word;
		card.suit = word;

		// cards
		deck.push_back(card);
	}

	file.close();
	return deck;
}

bool isNumber(string number)
{
	try
	{
		int num = stoi(number);
	}
	catch (invalid_argument)
	{
		return false;
	}

	return true;
}

bool compareContourArea(vector<Point> i, vector<Point> j)
{
	// second parameter controls the orientation / closed section
	// "true" avoids duplicates when sorting
	return contourArea(i, true) > contourArea(j, true);
}

void appendToMat(Mat image, Mat section, int x, int y)
{
	for (int i = 0; i < section.rows; i++)
	{
		for (int j = 0; j < section.cols; j++)
		{
			Vec3b pixel = section.at<Vec3b>(i, j);
			image.at<Vec3b>(i + y, j + x) = pixel;
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

int getCardDiff(Mat detectedCard, Mat deckCard)
{
	Mat diff;

	absdiff(detectedCard, deckCard, diff);
	GaussianBlur(diff, diff, Size(5, 5), 5);
	threshold(diff, diff, 200, 255, THRESH_BINARY);

	/*namedWindow("detectedCard");
	imshow("detectedCard", detectedCard);
	namedWindow("deckCard");
	imshow("deckCard", deckCard);
	namedWindow("diff");
	imshow("diff", diff);
	waitKey(0);*/

	return countNonZero(diff);
}

vector<vector<Point>> getContours(Mat image, int nCards)
{
	Mat processing;
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	vector<Point2f[4]> rectangles;

	// grayscale, blur, threshold
	cvtColor(image, processing, CV_BGR2GRAY);
	GaussianBlur(processing, processing, Size(1, 1), 1000);
	threshold(processing, processing, 120, 255, THRESH_BINARY);

	// edge detection and contours
	Canny(processing, processing, 0, 60, 3);
	findContours(processing, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// sorting by largest area
	sort(contours.begin(), contours.end(), compareContourArea);

	return contours;
}

Rectangle getCardRectangle(vector<Point> contour)
{
	RotatedRect rotatedRect = minAreaRect(contour);
	Point2f rectPoints[4];

	rotatedRect.points(rectPoints);

	if (calculateDistance(rectPoints[0], rectPoints[1]) < calculateDistance(rectPoints[1], rectPoints[2]))
	{
		Point2f p0 = rectPoints[0];
		rectPoints[0] = rectPoints[1];
		rectPoints[1] = rectPoints[2];
		rectPoints[2] = rectPoints[3];
		rectPoints[3] = p0;
	}

	return Rectangle{ rectPoints[0], rectPoints[1], rectPoints[2], rectPoints[3] };
}

Mat getCardPerspective(Mat image, Rectangle rectangle)
{
	Mat perspective;
	Point2f transformPoints[4];
	Point2f rectanglePoints[] = { rectangle.p1, rectangle.p2, rectangle.p3, rectangle.p4 };

	transformPoints[0] = Point2f(0, 449);
	transformPoints[1] = Point2f(0, 0);
	transformPoints[2] = Point2f(449, 0);
	transformPoints[3] = Point2f(449, 449);

	Mat transform = getPerspectiveTransform(rectanglePoints, transformPoints);
	warpPerspective(image, perspective, transform, Size(450, 450));
	preprocess(perspective);

	return perspective;
}

Card detectCard(Mat perspective, vector<Card> deck, Mat deckImage)
{
	Mat flipped;
	flip(perspective, flipped, -1);

	int bestDiff = INT_MAX;
	int bestIndex;

	for (size_t i = 0; i < deck.size(); i++)
	{
		Mat deckCard = deckImage(CvRect(i * 450, 0, 450, 450));

		int diff = getCardDiff(perspective, deckCard);
		int flippedDiff = getCardDiff(flipped, deckCard);
		int minDiff = min(diff, flippedDiff);

		if (minDiff < bestDiff)
		{
			bestDiff = minDiff;
			bestIndex = i;
		}
	}

	return deck[bestIndex];
}

void train(string filename, int nCards)
{
	Mat deck = imread(filename, IMREAD_COLOR);

	if (deck.empty())
	{
		cout << "Could not open or find the image" << endl;
		return;
	}

	Mat cardBase = Mat::zeros(Size(450 * nCards, 450), CV_8UC3);
	vector<vector<Point>> contours = getContours(deck, nCards);

	if ((int)contours.size() < nCards)
	{
		return;
	}

	for (int i = 0; i < nCards; i++)
	{
		Rectangle rectangle = getCardRectangle(contours[i]);
		Mat perspective = getCardPerspective2(deck, rectangle);

		appendToMat(cardBase, perspective, 450 * i, 0);
	}

	imwrite("../Assets/output.png", cardBase);
}

/********************************************************************
* TO DO
********************************************************************/

Mat getCardPerspective2(Mat image, Rectangle rectangle)
{
	Mat perspective;
	Point2f transformPoints[4];
	Point2f rectanglePoints[] = { rectangle.p1, rectangle.p2, rectangle.p3, rectangle.p4 };

	transformPoints[0] = Point2f(0, 449);
	transformPoints[1] = Point2f(0, 0);
	transformPoints[2] = Point2f(449, 0);
	transformPoints[3] = Point2f(449, 449);

	Mat transform = getPerspectiveTransform(rectanglePoints, transformPoints);
	warpPerspective(image, perspective, transform, Size(450, 450));

	return perspective;
}

int useSurf(Mat detectedCard, Mat deckCard)
{
	//-- Step 1: Detect the keypoints using SURF Detector
	/*int minHessian = 400;

	SurfFeatureDetector detector(minHessian);
	vector<KeyPoint> keypoints_1, keypoints_2;

	detector.detect(detectedCard, keypoints_1);
	detector.detect(deckCard, keypoints_2);

	//-- Step 2: Calculate descriptors (feature vectors)
	SurfDescriptorExtractor extractor;
	Mat descriptors_1, descriptors_2;

	extractor.compute(detectedCard, keypoints_1, descriptors_1);
	extractor.compute(deckCard, keypoints_2, descriptors_2);

	//-- Step 3: Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	double max_dist = 0; double min_dist = 100;
	vector< DMatch > matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_1.rows; i++)
	{
		double dist = matches[i].distance;
	}

	//-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist, or a small arbitary value ( 0.02 ) in the event that min_dist is very small)
	vector< DMatch > good_matches;

	for (int i = 0; i < descriptors_1.rows; i++)
	{
		if (matches[i].distance <= max(2 * min_dist, 0.02))
		{
			good_matches.push_back(matches[i]);
		}
	}

	return (int) matches.size();*/
	return 0;
}

Card detectCard2(Mat perspective, vector<Card> deck, Mat deckImage)
{
	Mat flipped;
	flip(perspective, flipped, -1);

	int bestDiff = INT_MAX;
	int bestIndex;

	for (size_t i = 0; i < deck.size(); i++)
	{
		Mat deckCard = deckImage(CvRect(i * 450, 0, 450, 450));

		int diff = useSurf(perspective, deckCard);
		int flippedDiff = useSurf(flipped, deckCard);
		int minDiff = min(diff, flippedDiff);

		if (minDiff < bestDiff)
		{
			bestDiff = minDiff;
			bestIndex = i;
		}
	}

	return deck[bestIndex];
}
