#include "CardDetection.h"

void train(string filename, int nCards, DetectionMethod method)
{
	Mat deck = imread(filename, IMREAD_COLOR);

	if (deck.empty())
	{
		cout << "Could not open or find the file." << endl;
		exit(-1);
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

	imwrite("../Assets/deck_training.png", cardBase);
}

vector<Card> readDeckList(string filename)
{
	ifstream file(filename);
	stringstream stream;
	string line, word;
	vector<Card> deck;
	Card card;

	if (!file.is_open())
	{
		cout << "Could not open or find the file." << endl;
		exit(-1);
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

void readDeckImage(string filename, vector<Card> &deck, DetectionMethod method)
{
	Mat deckImage;

	if (method == Binary)
	{
		deckImage = imread(filename, IMREAD_GRAYSCALE);
	}

	else if (method == Surf)
	{
		deckImage = imread(filename, IMREAD_COLOR);
	}

	if (deckImage.empty())
	{
		cout << "Could not open or find the file." << endl;
		exit(-1);
	}

	SurfFeatureDetector detector(400);
	SurfDescriptorExtractor extractor;

	for (size_t i = 0; i < deck.size(); i++)
	{
		Mat card = deckImage(Rect(i * 450, 0, 450, 450));
		deck[i].image = card;

		if (method == Surf)
		{
			vector<KeyPoint> keyPoints;
			Mat descriptors;

			detector.detect(card, keyPoints);
			extractor.compute(card, keyPoints, descriptors);

			deck[i].keyPoints = keyPoints;
			deck[i].descriptors = descriptors;
		}	
	}
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

	return countNonZero(diff);
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

int getSurfMatches(vector<KeyPoint> keyPoints1, Mat descriptors1, vector<KeyPoint> keyPoints2, Mat descriptors2)
{
	FlannBasedMatcher matcher;
	double maxDist = 0.2;
	vector<DMatch> matches;

	matcher.match(descriptors1, descriptors2, matches);
	filterMatchesByAbsoluteValue(matches, maxDist);
	filterMatchesRANSAC(matches, keyPoints1, keyPoints2);

	return (int)matches.size();
}

int detectCardBinary(Mat card, Mat flipped, vector<Card> deck)
{
	int bestDiff = INT_MAX;
	int bestIndex;

	for (size_t i = 0; i < deck.size(); i++)
	{
		int diff = getBinaryDiff(card, deck[i].image);
		int flippedDiff = getBinaryDiff(flipped, deck[i].image);

		int minDiff = min(diff, flippedDiff);

		if (minDiff < bestDiff)
		{
			bestDiff = minDiff;
			bestIndex = i;
		}
	}

	return bestIndex;
}

int detectCardSurf(Mat card, Mat flipped, vector<Card> deck)
{
	int bestMatches = -1;
	int bestIndex;

	SurfFeatureDetector detector(400);
	SurfDescriptorExtractor extractor;
	vector<KeyPoint> keyPoints, keyPointsFlipped;
	Mat descriptors, descriptorsFlipped;

	detector.detect(card, keyPoints);
	extractor.compute(card, keyPoints, descriptors);

	for (size_t i = 0; i < deck.size(); i++)
	{
		int matches = getSurfMatches(keyPoints, descriptors, deck[i].keyPoints, deck[i].descriptors);
		int flipedMatches = getSurfMatches(keyPointsFlipped, descriptorsFlipped, deck[i].keyPoints, deck[i].descriptors);

		int maxMatches = max(matches, flipedMatches);

		if (maxMatches > bestMatches)
		{
			bestMatches = maxMatches;
			bestIndex = i;
		}
	}

	return bestIndex;
}

Card detectCard(Mat card, vector<Card> deck, DetectionMethod method)
{
	int cardIndex = 0;
	Mat flipped;
	flip(card, flipped, -1);

	if (method == Binary)
	{
		cardIndex = detectCardBinary(card, flipped, deck);
	}
	else if (method == Surf)
	{
		cardIndex = detectCardSurf(card, flipped, deck);
	}

	return deck[cardIndex];
}