#include <iostream>
#include "CardDetection.h"

using namespace std;

string baseAssetsPath = "../Assets/";

void displayHelp();
void detectCards(string deckImagePath, string deckListPath);

int main(int argc, char** argv)
{
	// train(baseAssetsPath + "cards.png", 54);
	detectCards(baseAssetsPath + "deck.png", baseAssetsPath + "deck.txt");
	waitKey(0);
}

void detectCards(string deckImagePath, string deckListPath)
{
	Mat image = imread("../Assets/3.jpg", IMREAD_COLOR);
	Mat deckImage = imread(deckImagePath, IMREAD_GRAYSCALE); // IMREAD_GRAYSCALE
	vector<Card> deck = readDeck(deckListPath);
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