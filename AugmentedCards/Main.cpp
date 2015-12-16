#include <iostream>
#include "CardDetection.h"

using namespace std;

void detectCards();

int main(int argc, char** argv)
{
	detectCards();
	waitKey(0);
}

void detectCards()
{
	Mat image = imread("../Assets/3.jpg", IMREAD_COLOR);
	Mat deckImage = imread("../Assets/cardbase.png", IMREAD_GRAYSCALE);
	vector<Card> deck = readDeck("../Assets/deck.txt");
	int nCards = 4;

	if (image.empty())
	{
		cout << "Could not open or find the image" << endl;
		return;
	}

	vector<vector<Point>> contours = getContours(image, nCards);

	if ((int)contours.size() < nCards)
	{
		return;
	}

	for (int i = 0; i < nCards; i++)
	{
		Rectangle rectangle = getCardRectangle(contours[i]);
		Mat perspective = getCardPerspective(image, rectangle);
		Card card = detectCard(perspective, deck, deckImage);

		// testing
		cout << "Matched with " << card.symbol << " | " << card.suit << endl;

		vector<Vec4i> hierarchy;

		drawContours(image, contours, i, Scalar(0,0,255), 1, CV_AA, hierarchy, 0, Point());

		for (int j = 0; j < 4; j++)
		{
			Point2f rectanglePoints[] = { rectangle.p1, rectangle.p2, rectangle.p3, rectangle.p4 };
			line(image, rectanglePoints[j], rectanglePoints[(j + 1) % 4], Scalar(255, 0, 0), 3, 8);
		}
	}

	namedWindow("main");
	imshow("main", image);
}