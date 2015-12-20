#pragma once

#include <opencv\cv.h>
#include <opencv2\core\core_c.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\nonfree\nonfree.hpp>
#include <opencv2\nonfree\features2d.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <limits>

#include "Card.h"
#include "DetectionMethod.h"
#include "Lines.h"
#include "Rectangle.h"

using namespace cv;
using namespace std;

/*
 * General methods for card detection.
 */

const int SURF_HESSIAN = 600;
const double SURF_MAX_DIST = 0.125;
const double RANSAC_THRESHOLD = 3;

/* Generates and stores a deck (as image) to disk. */
void train(string filename, int nCards, DetectionMethod method);

/* Reads a file containing all the cards (as pairs of symbols/suits) in a deck. */
vector<Card> readDeckList(string filename);

/* Reads an image containing all the cards in a deck and appends each card, as an image, to an existing vector. */
void readDeckImage(string filename, vector<Card> &deck, DetectionMethod method);

/* Checks whether a given string is a number. */
bool isNumber(string number);

/* Returns all the contours in an image ordered by largest area. */
vector<vector<Point>> getContours(Mat image);

/* Auxiliar to getContours, used for sorting a vector by the area of a set of points. */
bool compareContourArea(vector<Point> v1, vector<Point> v2);

/* Appends an image to another at a specific section. The section to be appended should be smaller than the image it is appended to. */
void appendToMat(Mat image, Mat section, int x, int y);

/* Pastes an image on top of another. Black sections in the second image are treated as a mask, and are ignored. */
void copyTransparent(Mat &image1, Mat image2);

/* Returns the distance between two points. */
float calculateDistance(Point2f p1, Point2f p2);

/* Returns the distance between two points. */
double getAngleBetweenPoints(Point pt1, Point pt2);

/* Returns the four points representing a rectangle in a list of points defining a closed section (contours). 
 * Uses OpenCV's internal functions (minAreaRect). */
Rectangle getCardRectangle(vector<Point> contour);

/* Returns the four points representing a rectangle in a list of points defining a closed section (contours).
 * Uses the distance and angle between pairs of points. */
Rectangle getCardRectangleByDiagonals(vector<Point> contour);

/* Returns the four points representing a rectangle in a list of points defining a closed section (contours).
 * Uses the equations for the lines formed between pairs of points. */
Rectangle getCardRectangleByEquation(vector<Point> contour);

/* Pre-processing applied to each card during the binary method.
 * Black and white -> blur -> threshold. Removes noise and provides better contours. */
void binaryPreprocess(Mat &image);

/* Converts the section formed by a rectangle (card) to a new image with a warping processing. */
Mat getCardPerspective(Mat image, Rectangle rectangle, DetectionMethod method);

/* Given an image of a card and a deck, returns the closest match. */
Card detectCard(Mat perspective, vector<Card> deck, DetectionMethod method);

/* Auxiliar to detectCard, attempts to match cards using the Binary method. */
int detectCardBinary(Mat card, Mat flipped, vector<Card> deck);

/* Auxiliar to detectCardBinary, returns the number of differences between two images in pixels. */
int getBinaryDiff(Mat detectedCard, Mat deckCard);

/* Auxiliar to detectCard, attempts to match cards using the SURF method. */
int detectCardSurf(Mat card, vector<Card> deck);

/* Auxiliar to detectCardSurf, returns the number of matches between two images in pixels. */
int getSurfMatches(vector<KeyPoint> keyPoints1, Mat descriptors1, vector<KeyPoint> keyPoints2, Mat descriptors2);

/* Auxiliar to getSurfMatches, filters matches by distance. */
void filterMatchesByAbsoluteValue(std::vector<DMatch> &matches, float maxDistance);

/* Auxiliar to getSurfMatches, filters matches using RANSAC. */
Mat filterMatchesRANSAC(vector<DMatch> &matches, vector<KeyPoint> &keypointsA, vector<KeyPoint> &keypointsB, double threshold);

/* Draws card values, contours and defining points (rectangle) in a given image. */
Mat drawCards(Mat image, vector<Card> move, vector<int> winners);

/* Draws the value for a given card in an image. */
Mat drawCardValue(Mat image, Card card, bool winner);

/* Draws text in an image, centered within a point. */
Mat drawTextCentered(Mat image, Point center, string text, Scalar color);

/* Draws the contours for a given card in an image. */
Mat drawCardContours(Mat image, Card card, bool winner);

/* Draws the rectangle (points and lines) for a given card in an image. */
Mat drawCardRectangle(Mat image, Card card);

/* Resizes an image within the given limits. Maintains proportions. */
Mat resizeWithLimits(Mat image, int width, int height);