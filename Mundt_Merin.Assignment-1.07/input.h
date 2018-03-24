#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <string.h>

using namespace std;



class npc_template_t{
	public:
	bool invalid = false;	
	string symbol;
	string description;
	string name;
	string color;
	string speed;
	string abilities;
	string hitpoints;
	string attackDamage;
	int rarity = -1;
};
#endif

