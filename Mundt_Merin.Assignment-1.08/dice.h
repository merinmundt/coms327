#ifndef Dice_h
#define Dice_h

#include <string>
#include <string.h>

using namespace std;

class dice{
public:
    int numOfDice;
    int numOfSides;
    int base;

    dice();
    dice(string dicestring);
    int roll();


};
#endif
