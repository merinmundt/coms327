#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <string.h>
#include <vector>
#include "dungeon.h"

using namespace std;



class npc_template_t{
	public:
	bool invalid = false;	
	string symbol;
	string description;
	string name;
	string color;
	string speed;
	string abilities = "empty";
	string hitpoints;
	string attackDamage;
	int rarity = -1;
	bool isvalid();
	void print();
	npc_t generate();
	bool isUnique();

};

class object_template_t{
public:
	string name;
	string description;
	string type;
	string color;
	string hitbonus;
	string damagebonus;
	string dodgebonus;
	string defensebonus;
	string weight;
	string speedbonus;
	string specialattribute;
	string value;
	string artifactstatus;
	int rarity = 0;
	bool invalid = false;
	bool isvalid();
	void print();
	game_object_t generate();

};

vector<npc_template_t> parseMonsterTemplates(string filename);
vector<object_template_t> parseObjectTemplates(string filename);

#endif

