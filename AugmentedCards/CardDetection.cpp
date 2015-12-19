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
		cout << "Number of cards required for training not found." << endl;
		exit(-1);
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

vector<Card> readDeckList(string path)
{
	ifstream file(path + "deck.txt");
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

void readDeckImage(string path, vector<Card> &deck, DetectionMethod method)
{
	Mat deckImage;

	if (method == Binary)
	{
		deckImage = imread(path + "deck_binary.png", IMREAD_GRAYSCALE);
	}

	else if (method == Surf)
	{
		deckImage = imread(path + "deck_surf.png", IMREAD_COLOR);
		cout << endl << "Pre-processing the deck..." << endl;
	}

	if (deckImage.empty())
	{
		cout << "Could not open or find the file." << endl;
		exit(-1);
	}

	SurfFeatureDetector detector(SURF_HESSIAN);
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

void copyTransparent(Mat &image1, Mat image2)
{
	for (int i = 0; i < image2.size().height; i++)
	{
		for (int j = 0; j < image2.size().width; j++)
		{
			Vec3b pixel = image2.at<Vec3b>(i, j);

			if (pixel[0] != 0 || pixel[1] || pixel[2] != 0)
			{
				image1.at<Vec3b>(i, j) = pixel;
			}
		}
	}
}

float calculateDistance(Point2f p1, Point2f p2)
{
	float diffY = p1.y - p2.y;
	float diffX = p1.x - p2.x;
	return sqrt((diffY * diffY) + (diffX * diffX));
}

double getAngleBetweenPoints(Point pt1, Point pt2)
{
	double xDiff = pt2.x - pt1.x;
	double yDiff = pt2.y - pt1.y;

	if (atan2(yDiff, xDiff) < 0)
	{
		return atan2(yDiff, xDiff) + 2 * CV_PI;
	}
	else
	{
		return atan2(yDiff, xDiff);
	}
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

Rectangle getCardRectangleByDiagonals(vector<Point> contour)
{
	vector<Point> poly;
	approxPolyDP(contour, poly, 1, true);

	set<Line, CompareLineDistance> sortByDistance;

	for (int i = 0; i < poly.size() - 1; i++)
	{
		for (int j = i + 1; j < poly.size(); j++)
		{
			Line line;
			line.p1 = poly[i];
			line.p2 = poly[j];
			line.distance = calculateDistance(line.p1, line.p2);

			sortByDistance.insert(line);
		}
	}

	set<Line>::reverse_iterator it;
	it = sortByDistance.rbegin();

	Line l1 = *it++;
	Line l2;
	float l1a = getAngleBetweenPoints(l1.p1, l1.p2);

	while (it != sortByDistance.rend())
	{
		Line line = *it++;
		float linea = getAngleBetweenPoints(line.p1, line.p2);

		float angDiff = abs(linea - l1a);

		if (angDiff > (CV_PI / 3) && angDiff < (2 * CV_PI / 3))
		{
			l2 = line;
			break;
		}
	}

	return Rectangle{ l1.p1, l2.p1, l1.p2, l2.p2 };
}

Rectangle getCardRectangleByEquation(vector<Point> contour)
{
	//reduce the number of points
	vector<Point> poly;

	approxPolyDP(contour, poly, 3, true);
	poly.push_back(poly[0]); //this is done so that last point and first point are considered a line

	//create a sort-by-distance set of Line
	set<Line, CompareLineDistance> sortbydistance;

	//for each consequent two points, create a Line and add it
	for (int i = 0; i < poly.size() - 1; i++)
	{
		Point2f p1 = poly[i];
		Point2f p2 = poly[i + 1];

		float distance = calculateDistance(p1, p2);

		Line line;
		line.seq = i;
		line.p1 = p1;
		line.p2 = p2;
		line.distance = distance;

		sortbydistance.insert(line);
	}

	//create a sort-by-sequence set of Line
	set<Line, CompareLineSequence> sortbysequence;

	//get the best 4 lines (card sides) and add them
	set<Line>::iterator dstIt;
	dstIt = sortbydistance.end();

	for (int i = 0; i < 4; i++)
	{
		sortbysequence.insert(*(--dstIt));
	}

	// we will need to sequently calculate intersections (1,2) (2,3) (3,4) (4,1)
	// so to make it easier:
	// store lines in a vector
	// add 1 in the end too
	vector<Point2f> cardcorners;

	vector<Line> cardsides;
	set<Line>::iterator seqIt;

	for (seqIt = sortbysequence.begin(); seqIt != sortbysequence.end(); seqIt++)
		cardsides.push_back(*(seqIt));

	cardsides.push_back(cardsides[0]);

	for (int i = 0; i < cardsides.size() - 1; i++)
	{
		Line l1 = cardsides[i];
		Line l2 = cardsides[i + 1];

		//calculate l1 equation
		bool l1v = false;
		float l1x;
		float l1m;
		float l1b;

		if ((l1.p2.x - l1.p1.x) == 0) //vertical line
		{
			l1x = l1.p1.x;
			l1v = true;
		}
		else
		{
			l1m = (l1.p2.y - l1.p1.y) / (l1.p2.x - l1.p1.x);
			l1b = (l1.p1.y - l1m * l1.p1.x);
		}

		//calculate l2 equation
		bool l2v = false;
		float l2x;
		float l2m;
		float l2b;

		if ((l2.p2.x - l2.p1.x) == 0) //vertical line
		{
			l2x = l2.p1.x;
			l2v = true;
		}
		else
		{
			l2m = (l2.p2.y - l2.p1.y) / (l2.p2.x - l2.p1.x);
			l2b = (l2.p1.y - l2m * l2.p1.x);
		}

		//calculate intersection
		float x;
		float y;

		if (!l1v && !l2v) //no verticals
		{
			x = (l2b - l1b) / (l1m - l2m);
			y = l1m*x + l1b;
		}
		else if (l1v) //l1 vertical
		{
			x = l1x;
			y = l2m*x + l2b;
		}
		else if (l2v) //l2 vertical
		{
			x = l2x;
			y = l1m*x + l1b;
		}

		cardcorners.push_back(Point2f(x, y));
	}

	//Create a rectangle
	Rectangle cardRectangle = Rectangle{ cardcorners[0], cardcorners[1], cardcorners[2], cardcorners[3] };

	if (calculateDistance(cardRectangle.p1, cardRectangle.p2) < calculateDistance(cardRectangle.p2, cardRectangle.p3))
	{
		Point2f p1 = cardRectangle.p1;
		cardRectangle.p1 = cardRectangle.p2;
		cardRectangle.p2 = cardRectangle.p3;
		cardRectangle.p3 = cardRectangle.p4;
		cardRectangle.p4 = p1;
	}

	return cardRectangle;
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

	for (size_t i = 0; i < matches.size(); i++)
	{
		if (matches[i].distance < maxDistance)
		{
			filteredMatches.push_back(matches[i]);
		}
	}

	matches = filteredMatches;
}

Mat filterMatchesRANSAC(vector<DMatch> &matches, vector<KeyPoint> &keypointsA, vector<KeyPoint> &keypointsB, double threshold)
{
	Mat homography;
	vector<DMatch> filteredMatches;

	if (matches.size() >= 4)
	{
		vector<Point2f> srcPoints;
		vector<Point2f> dstPoints;
		for (size_t i = 0; i < matches.size(); i++)
		{

			srcPoints.push_back(keypointsA[matches[i].queryIdx].pt);
			dstPoints.push_back(keypointsB[matches[i].trainIdx].pt);
		}

		Mat mask;
		homography = findHomography(srcPoints, dstPoints, CV_RANSAC, threshold, mask);

		for (int i = 0; i<mask.rows; i++)
		{
			if (mask.ptr<uchar>(i)[0] == 1)
			{ 
				filteredMatches.push_back(matches[i]);
			}
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
	filterMatchesByAbsoluteValue(matches, SURF_MAX_DIST);
	filterMatchesRANSAC(matches, keyPoints1, keyPoints2, RANSAC_THRESHOLD);

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

	SurfFeatureDetector detector(SURF_HESSIAN);
	SurfDescriptorExtractor extractor;
	vector<KeyPoint> keyPoints, keyPointsFlipped;
	Mat descriptors, descriptorsFlipped;

	detector.detect(card, keyPoints);
	extractor.compute(card, keyPoints, descriptors);

	for (size_t i = 0; i < deck.size(); i++)
	{
		int matches = getSurfMatches(keyPoints, descriptors, deck[i].keyPoints, deck[i].descriptors);

		if (matches > bestMatches)
		{
			bestMatches = matches;
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

Mat drawCards(Mat image, vector<Card> move, vector<int> winners)
{
	for (size_t i = 0; i < move.size(); i++)
	{
		bool winner = find(winners.begin(), winners.end(), i) != winners.end();

		image = drawCardRectangle(image, move[i]);
		image = drawCardValue(image, move[i], winner);
		image = drawCardContours(image, move[i], winner);		
	}

	return image;
}

Mat drawCardValue(Mat image, Card card, bool winner)
{
	string text = card.symbol + " " + card.suit;
	Scalar color = winner ? Scalar(0, 255, 0) : Scalar(0, 0, 255);

	Mat tmpCard = Mat::zeros(450, 450, image.type());
	Mat tmpImage = Mat::zeros(image.size(), image.type());

	Point2f rectanglePoints[] = { card.rectangle.p1, card.rectangle.p2, card.rectangle.p3, card.rectangle.p4 };
	Point2f transformPoints[4];
	transformPoints[0] = Point2f(0, 449);
	transformPoints[1] = Point2f(0, 0);
	transformPoints[2] = Point2f(449, 0);
	transformPoints[3] = Point2f(449, 449);

	tmpCard = drawTextCentered(tmpCard, Point(225, 225), text, color);

	Mat transform = getPerspectiveTransform(transformPoints, rectanglePoints);
	warpPerspective(tmpCard, tmpImage, transform, image.size());
	
	copyTransparent(image, tmpImage);
	return image;
}

Mat drawTextCentered(Mat image, Point center, string text, Scalar color)
{
	int fontFace = FONT_HERSHEY_TRIPLEX;
	int scale = 2;
	int thickness = 3;

	Size textSize = getTextSize(text, fontFace, scale, thickness, 0);
	Point textPoint = Point(center.x - (textSize.width / 2), center.y + (textSize.height / 2));
	putText(image, text, textPoint, fontFace, scale, color, thickness, CV_AA);

	return image;
}

Mat drawCardContours(Mat image, Card card, bool winner)
{
	Scalar color = winner ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
	int thickness = 3;

	for (int i = 0; i < card.contours.size(); i++)
	{
		line(image, card.contours[i], card.contours[(i + 1) % card.contours.size()], color, thickness, CV_AA);
	}

	return image;
}

Mat drawCardRectangle(Mat image, Card card)
{
	Point2f rectanglePoints[] = { card.rectangle.p1, card.rectangle.p2, card.rectangle.p3, card.rectangle.p4 };

	Scalar color = Scalar(255, 0, 255);
	double circleRadius = 6;
	int circleThickness = 2;
	int lineThickness = 1;

	for (int i = 0; i < 4; i++)
	{
		circle(image, rectanglePoints[i], circleRadius, color, circleThickness, CV_AA);
		line(image, rectanglePoints[i], rectanglePoints[(i + 1) % 4], color, lineThickness, CV_AA);
	}

	return image;
}

Mat resizeWithLimits(Mat image, int width, int height)
{
	if (image.size().width <= width && image.size().height <= height)
	{
		return image;
	}

	float wRatio = (float)image.size().width / width;
	float hRatio = (float)image.size().height / height;
	int newWidth;
	int newHeight;
	Mat resized;

	if (wRatio >= hRatio)
	{
		newWidth = width;
		newHeight = (int)(image.size().height / wRatio);
	}
	else
	{
		newHeight = height;
		newWidth = (int)(image.size().width / hRatio);
	}

	resize(image, resized, Size(newWidth, newHeight));
	return resized;
}