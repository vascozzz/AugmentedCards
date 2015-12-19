#pragma once

#include <iostream>
#include <vector>
#include "Card.h"

using namespace std;

/*
 * Abstract representation of a game. 
 * All games should be able to evaluate a move (multiple cards)
   and return a vector with the indexes of the winning cards.
 */
class CardGame
{
private:
	virtual int getCardValue(Card card) = 0;

public:
	CardGame();
	~CardGame();
	virtual vector<int> evaluateGame(vector<Card> move) = 0;
};

