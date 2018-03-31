#include "dice.h"
#include <sstream>
#include <string.h>

using namespace std;

///returns false is string is not an int, otherwise sets the number to the int reprsented by the string.
static bool parseInt(string input, int &number){
    stringstream convertor;
    convertor << input;
    convertor >> number;

    if(convertor.fail()){
        return false;
    }

    return true;
}

dice::dice(){
    base = 0;
    numOfSides = 0;
    numOfDice = 0;

}

dice::dice(string dicestring){
    stringstream ss(dicestring);
    string dicenum;
    getline(ss, dicenum, '+');
    if(!parseInt(dicenum, base)){
        base = 0;
    }
    getline(ss, dicenum, 'd');
    if(!parseInt(dicenum, numOfDice)){
        numOfDice = 0;
    }
    getline(ss,dicenum);
    if(!parseInt(dicenum, numOfSides)){
        numOfSides = 0;
    }
}

int dice::roll(){
    //base + roll of all sides
    int result = 0;
    for ( int i = 0; i < numOfDice; i++){
        result += rand() % numOfSides + 1;
    }
    result += base;
    return result;
}

