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

/********************************************************************
* TO DO
********************************************************************/

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
	if ((int)contours.size() < nCards)
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
		appendToMat(cardbase, cardArray[i], 450 * i, 0);
	}


	//pre-process each card
	preprocess(cardbase);

	imshow("final", cardbase);
	imwrite("../Assets/cardbase.png", cardbase);

	waitKey(0);
	return 0;
}
