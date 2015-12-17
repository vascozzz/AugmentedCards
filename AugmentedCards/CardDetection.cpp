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

bool compareContourArea(vector<Point> v1, vector<Point> v2)
{
	// second parameter controls the orientation / closed section
	// "true" avoids duplicates when sorting
	return contourArea(v1, true) > contourArea(v2, true);
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

Mat getCardPerspective(Mat image, Rectangle rectangle, DetectionMethod method)
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

	if (method == Binary)
	{
		preprocess(perspective);
	}

	return perspective;
}

int getBinaryDiff(Mat detectedCard, Mat deckCard)
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

int getSurfMatches(Mat image1, Mat image2)
{
	int minHessian = 400;
	double maxDist = 0.2;

	SurfFeatureDetector detector(minHessian);
	SurfDescriptorExtractor extractor;
	FlannBasedMatcher matcher;

	vector<DMatch> matches;
	vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;

	// detect keypoints
	detector.detect(image1, keypoints1);
	detector.detect(image2, keypoints2);

	// clculate descriptors
	extractor.compute(image1, keypoints1, descriptors1);
	extractor.compute(image2, keypoints2, descriptors2);

	// matches
	matcher.match(descriptors1, descriptors2, matches);
	filterMatchesByAbsoluteValue(matches, maxDist);
	filterMatchesRANSAC(matches, keypoints1, keypoints2);

	/*Mat img_matches;
	drawMatches(detectedCard, keypoints_1, deckCard, keypoints_2, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	imshow("matches", img_matches);
	imshow("card", detectedCard);
	imshow("deck", deckCard);
	cout << "Good matches: " << matches.size() << endl;
	waitKey(0);*/

	return (int)matches.size();
}

void filterMatchesByAbsoluteValue(vector<DMatch> &matches, float maxDistance)
{
	vector<DMatch> filteredMatches;

	for (size_t i = 0; i<matches.size(); i++)
	{
		if (matches[i].distance < maxDistance)
		{
			filteredMatches.push_back(matches[i]);
		}
	}

	matches = filteredMatches;
}

Mat filterMatchesRANSAC(vector<DMatch> &matches, vector<KeyPoint> &keypointsA, vector<KeyPoint> &keypointsB)
{
	Mat homography;
	vector<DMatch> filteredMatches;

	if (matches.size() >= 4)
	{
		vector<Point2f> srcPoints;
		vector<Point2f> dstPoints;
		for (size_t i = 0; i<matches.size(); i++)
		{

			srcPoints.push_back(keypointsA[matches[i].queryIdx].pt);
			dstPoints.push_back(keypointsB[matches[i].trainIdx].pt);
		}

		Mat mask;
		homography = findHomography(srcPoints, dstPoints, CV_RANSAC, 1.0, mask);

		for (int i = 0; i<mask.rows; i++)
		{
			if (mask.ptr<uchar>(i)[0] == 1)
				filteredMatches.push_back(matches[i]);
		}
	}

	matches = filteredMatches;
	return homography;
}

Card detectCard(Mat perspective, vector<Card> deck, Mat deckImage, DetectionMethod method)
{
	Mat flipped;
	flip(perspective, flipped, -1);

	int bestDiff;
	int bestIndex;

	if (method == Binary)
	{
		bestDiff = INT_MAX;

		for (size_t i = 0; i < deck.size(); i++)
		{
			Mat deckCard = deckImage(Rect(i * 450, 0, 450, 450));

			int diff = getBinaryDiff(perspective, deckCard);
			int flippedDiff = getBinaryDiff(flipped, deckCard);
			int minDiff = min(diff, flippedDiff);

			if (minDiff < bestDiff)
			{
				bestDiff = minDiff;
				bestIndex = i;
			}
		}
	}

	else if (method == Surf)
	{
		bestDiff = -1;

		for (size_t i = 0; i < deck.size(); i++)
		{
			Mat deckCard = deckImage(Rect(i * 450, 0, 450, 450));

			int diff = getSurfMatches(perspective, deckCard);
			int flippedDiff = getSurfMatches(flipped, deckCard);
			int maxDiff = max(diff, flippedDiff);

			if (maxDiff > bestDiff)
			{
				bestDiff = maxDiff;
				bestIndex = i;
			}
		}
	}

	return deck[bestIndex];
}

void train(string filename, int nCards, DetectionMethod method)
{
	Mat deck = imread(filename, IMREAD_COLOR);

	if (deck.empty())
	{
		cout << "Could not open or find the image." << endl;
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
		Mat perspective;
		
		if (method == Binary)
		{
			perspective = getCardPerspective(deck, rectangle, Binary);
		}

		else if (method == Surf)
		{
			perspective = getCardPerspective(deck, rectangle, Surf);
		}

		appendToMat(cardBase, perspective, 450 * i, 0);
	}

	imwrite("../Assets/output.png", cardBase);
}
