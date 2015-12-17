#include <iostream>
#include "CardDetection.h"

using namespace std;

string baseAssetsPath = "../Assets/";

void displayHelp();
void detectCards(string deckImagePath, string deckListPath, DetectionMethod method);

int main(int argc, char** argv)
{
	// train(baseAssetsPath + "deck.png", 54, Surf);
	detectCards(baseAssetsPath + "deck.txt", baseAssetsPath + "deck_surf.png", Surf);
	waitKey(0);
}

void detectCards(string deckListPath, string deckImagePath, DetectionMethod method)
{
	Mat image;
	vector<Card> deck;
	vector<vector<Point>> contours;
	int nCards = 4;

	// read image
	image = imread("../Assets/4.jpg", IMREAD_COLOR);

	if (image.empty())
	{
		cout << "Could not open or find the file." << endl;
		return;
	}

	// read deck list
	deck = readDeckList(deckListPath);
	readDeckImage(deckImagePath, deck, method);

	// card detection
	contours = getContours(image, nCards);

	if ((int)contours.size() < nCards)
	{
		return;
	}

	for (int i = 0; i < nCards; i++)
	{
		Rectangle rectangle = getCardRectangle(contours[i]);
		Mat perspective = getCardPerspective(image, rectangle, method);
		Card card = detectCard(perspective, deck, method);

		cout << "Matched with " << card.symbol << " | " << card.suit << endl;
	}

	namedWindow("main");
	imshow("main", image);
}

void displayHelp()
{
	cout << "Usage example: " << endl;
	cout << "Available methods: " << endl;
}