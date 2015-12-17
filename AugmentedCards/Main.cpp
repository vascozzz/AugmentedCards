#include <iostream>
#include "CardDetection.h"

using namespace std;

string baseAssetsPath = "../Assets/";

void displayHelp();
void detectCards(string deckImagePath, string deckListPath, DetectionMethod method);

int main(int argc, char** argv)
{
	// train(baseAssetsPath + "deck.png", 54, Surf);
	detectCards(baseAssetsPath + "deck_surf.png", baseAssetsPath + "deck.txt", Surf);
	waitKey(0);
}

void detectCards(string deckImagePath, string deckListPath, DetectionMethod method)
{
	Mat image, deckImage;
	vector<Card> deck;
	vector<vector<Point>> contours;
	int nCards = 4;

	// read image
	image = imread("../Assets/4.jpg", IMREAD_COLOR);

	if (image.empty())
	{
		cout << "Could not open or find the image" << endl;
		return;
	}

	// read deck list
	deck = readDeck(deckListPath);

	// read deck image
	if (method == Binary)
	{
		deckImage = imread(deckImagePath, IMREAD_GRAYSCALE);
	}

	if (method == Surf)
	{
		deckImage = imread(deckImagePath, IMREAD_COLOR);
	}

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
		Card card = detectCard(perspective, deck, deckImage, method);

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