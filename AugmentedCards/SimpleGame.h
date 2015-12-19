#pragma once

#include <iostream>
#include <vector>
#include "Card.h"
#include "CardGame.h"

using namespace std;

/*
 * Simple game implementation. Highest value, regardless of suit, wins.
 */
class SimpleGame : public CardGame
{
private:
	virtual int getCardValue(Card card);

public:
	SimpleGame();
	~SimpleGame();
	virtual vector<int> evaluateGame(vector<Card> move);
};

