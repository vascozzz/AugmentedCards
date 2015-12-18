#include <iostream>
#include "CardDetection.h"
#include "SimpleGame.h"

using namespace std;


int gameCards = 2;
string baseAssetsPath = "../Assets/";
string baseDeckPath = baseAssetsPath + "deck/";


void displayIntro();
int parseDetectionMode();
DetectionMethod parseDetectionMethod();
Mat parseImage(string display);

void detectInImage(vector<Card> deck, DetectionMethod method);
void detectInVideo(vector<Card> deck, DetectionMethod method);
void detectCards(Mat image, vector<Card> deck, DetectionMethod method);

void playSimpleGame(vector<Card> move);


int main(int argc, char** argv)
{
	displayIntro();

	int detectionMode = parseDetectionMode();
	DetectionMethod detectionMethod = parseDetectionMethod();

	vector<Card> deck;
	deck = readDeckList(baseDeckPath);
	readDeckImage(baseDeckPath, deck, detectionMethod);

	switch (detectionMode)
	{
	case 1:
		detectInImage(deck, detectionMethod);
		break;
	case 2:
		detectInVideo(deck, detectionMethod);
		break;
	default:
		break;
	}

	waitKey(0);
}

void detectInImage(vector<Card> deck, DetectionMethod method)
{
	Mat image = parseImage("Select an image from the assets: ");
	namedWindow("Image");
	imshow("Image", image);
	detectCards(image, deck, method);
}

void detectInVideo(vector<Card> deck, DetectionMethod method)
{
	int keyPressed = 0;
	int captureKey = 13;
	int escapeKey = 27;

	VideoCapture cap = VideoCapture(0);
	Mat frame;
	namedWindow("Camera");

	if (!cap.isOpened())
	{
		cout << "Unable to start capture!" << endl;
		return;
	}

	while (cap.isOpened() && keyPressed != escapeKey)
	{
		keyPressed = waitKey(1);

		if (!cap.read(frame))
		{
			cout << "Skipped a frame..." << endl;
		}

		if (keyPressed == captureKey)
		{
			detectCards(frame, deck, method);
		}

		imshow("Camera", frame);
	}
}

void detectCards(Mat image, vector<Card> deck, DetectionMethod method)
{
	vector<Card> move;
	vector<vector<Point>> contours;

	contours = getContours(image, gameCards);

	if ((int)contours.size() < gameCards)
	{
		cout << "Couldn't detect the number of cards required to play the game!";
		return;
	}

	for (int i = 0; i < gameCards; i++)
	{
		Rectangle rectangle = getCardRectangle(contours[i]);
		Mat perspective = getCardPerspective(image, rectangle, method);
		Card card = detectCard(perspective, deck, method);

		card.rectangle = rectangle;
		move.push_back(card);
		cout << "Matched with " << card.symbol << " | " << card.suit << endl;
	}

	playSimpleGame(move);
	drawCards(image, move);
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

int parseDetectionMode()
{
	int choice;

	while (true)
	{
		cout << "Choose a mode: " << endl << endl;
		cout << "1 - Image" << endl;
		cout << "2 - Camera" << endl << endl;
		cout << "> ";
		cin >> choice;

		if (cin.fail())
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl << "Not a number! ";
		}
		else if (choice <= 0 || choice > 2)
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl << "Not a valid option! ";
		}
		else
		{
			break;
		}
	}

	return choice;
}

DetectionMethod parseDetectionMethod()
{
	int choice;
	DetectionMethod method;

	cout << endl;

	while (true)
	{
		cout << "Choose a method: " << endl << endl;
		cout << "1 - Binary" << endl;
		cout << "2 - SURF" << endl << endl;
		cout << "> ";
		cin >> choice;

		if (cin.fail())
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl << "Not a number! ";
		}
		else if (choice <= 0 || choice > 2)
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl << "Not a valid option! ";
		}
		else
		{
			if (choice == 1)
			{
				method = Binary;
			}
			else
			{
				method = Surf;
			}

			break;
		}
	}

	return method;
}

Mat parseImage(string display)
{
	string filename;
	Mat image;

	cout << endl;

	while (true)
	{
		cout << display << endl << endl;
		cout << "> ";
		cin >> filename;

		image = imread(baseAssetsPath + filename, IMREAD_COLOR);

		if (image.empty())
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << endl << "Could not open or find the image! ";
		}
		else
		{
			break;
		}
	}

	return image;
}

void displayIntro()
{
	cout << "################################################################" << endl;
	cout << "#                   Augmented Card Detection                   #" << endl;
	cout << "################################################################" << endl;
	cout << endl << endl;
}