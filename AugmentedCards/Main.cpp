#include <iostream>
#include "CardDetection.h"
#include "SimpleGame.h"

using namespace std;

const int GAME_CARDS = 4;

const string BASE_ASSETS_PATH = "../Assets/";
const string BASE_DECK_PATH = BASE_ASSETS_PATH + "deck/";

/* Displays the initial menu. */
void displayIntro();

/* Returns the detection mode requested by the user. */
int parseDetectionMode();

/* Returns the detection method requested by the user. */
DetectionMethod parseDetectionMethod();

/* Returns an image requested by the user. */
Mat parseImage(string display);

/* Attempts to detect cards in a given image. */
void detectInImage(vector<Card> deck, DetectionMethod method);

/* Attempts to detect cards using a camera. */
void detectInVideo(vector<Card> deck, DetectionMethod method);

/* Attemps to detect cards in a given frame. Draws the results for a simple game. */
void detectCards(Mat image, vector<Card> deck, DetectionMethod method);

int main(int argc, char** argv)
{
	displayIntro();

	int detectionMode = parseDetectionMode();
	DetectionMethod detectionMethod = parseDetectionMethod();

	vector<Card> deck;
	deck = readDeckList(BASE_DECK_PATH);
	readDeckImage(BASE_DECK_PATH, deck, detectionMethod);

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
	image = resizeWithLimits(image, 1000, 700);

	namedWindow("Image", WINDOW_AUTOSIZE);
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
	namedWindow("Camera", WINDOW_AUTOSIZE);

	if (!cap.isOpened())
	{
		cout << "Unable to start capture!" << endl;
		return;
	}

	cout << endl << "Press ENTER to capture a frame, and ESC to exit at any time." << endl;

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

	// Get image contours
	contours = getContours(image);

	if ((int)contours.size() < GAME_CARDS)
	{
		cout << endl << "Couldn't detect the number of cards required to play the game!";
		return;
	}

	// Process cards individually
	for (int i = 0; i < GAME_CARDS; i++)
	{
		Rectangle rectangle = getCardRectangleByEquation(contours[i]);
		Mat perspective = getCardPerspective(image, rectangle, method);
		Card card = detectCard(perspective, deck, method);

		card.contours = contours[i];
		card.rectangle = rectangle;
		move.push_back(card);

		cout << endl << "Matched with " << card.symbol << " | " << card.suit << endl;
	}

	// Evalute move
	SimpleGame game;
	vector<int> winners = game.evaluateGame(move);

	// Draw final result
	Mat detection = drawCards(image, move, winners);

	namedWindow("Detection", WINDOW_AUTOSIZE);
	imshow("Detection", image);
}

int parseDetectionMode()
{
	int choice;

	while (true)
	{
		cout << "Select a detection mode: " << endl << endl;
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
		cout << "Select a detection method: " << endl << endl;
		cout << "1 - Binary (fast)" << endl;
		cout << "2 - SURF (slower, better results)" << endl << endl;
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

		image = imread(BASE_ASSETS_PATH + filename, IMREAD_COLOR);

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
	cout << "                ___                                    __           __ " << endl;
	cout << "               /   | __  ______ _____ ___  ___  ____  / /____  ____/ / " << endl;
	cout << "              / /| |/ / / / __ '/ __ '__ \\/ _ \\/ __ \\/ __/ _ \\/ __  /  " << endl;
	cout << "             / ___ / /_/ / /_/ / / / / / /  __/ / / / /_/  __/ /_/ /   " << endl;
	cout << "            /_/  |_\\__,_/\\__, /_/ /_/ /_/\\___/_/ /_/\\__/\\___/\\__,_/    " << endl;
	cout << "                        /____/                                         " << endl;

	cout << "                          ______               __    " << endl;
	cout << "                         / ____/___ __________/ /____" << endl;
	cout << "                        / /   / __ '/ ___/ __  / ___/" << endl;
	cout << "                       / /___/ /_/ / /  / /_/ (__  ) " << endl;
	cout << "                       \\____/\\__,_/_/   \\__,_/____/ " << endl;

	cout << "\n________________________________________________________________________________\n\n";
	cout << "Welcome! This application will assist you while playing a simple card game.\n\n";
}