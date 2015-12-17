#include <iostream>
#include "CardDetection.h"
#include "SimpleGame.h"

using namespace std;

string baseAssetsPath = "../Assets/";

void detectCards(string deckImagePath, string deckListPath, DetectionMethod method);
void playSimpleGame(vector<Card> move);
void displayHelp();

int main(int argc, char** argv)
{
	// train(baseAssetsPath + "deck.png", 54, Surf);
	detectCards(baseAssetsPath + "deck.txt", baseAssetsPath + "deck_binary.png", Binary);
	waitKey(0);
}

void detectCards(string deckListPath, string deckImagePath, DetectionMethod method)
{
	Mat image;
	vector<Card> deck, move;
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

		move.push_back(card);
		cout << "Matched with " << card.symbol << " | " << card.suit << endl;
	}

	playSimpleGame(move);

	namedWindow("main");
	imshow("main", image);
}

void playSimpleGame(vector<Card> move)
{
	SimpleGame game;
	int winner = game.evaluateGame(move);

	if (winner < 0)
	{
		cout << "There was a tie between " << -winner << " players!";
	}
	else
	{
		cout << winner << " wins!";
	}
}

void displayHelp()
{
	cout << "Usage example: " << endl;
	cout << "Available methods: " << endl;
}