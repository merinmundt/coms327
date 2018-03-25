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
	string abilities = "empty";
	string hitpoints;
	string attackDamage;
	int rarity = -1;
	bool isvalid();
	void print();

};
vector<npc_template_t> parseMonsterTemplates(string filename);

#endif

