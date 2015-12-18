#pragma once

#include <iostream>
#include <vector>
#include "Card.h"

using namespace std;

class CardGame
{
private:
	virtual int getCardValue(Card card) = 0;

public:
	CardGame();
	~CardGame();
	virtual vector<int> evaluateGame(vector<Card> move) = 0;
};

