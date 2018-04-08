#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "input.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include "dice.h"
#include <ncurses.h>

using namespace std;

#define INTELLIGENT				1
#define TELEPATHIC				2
#define TUNNELLING				4
#define ERRATIC					8

static string readFile(const string &fileName)
{
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    string result = string(bytes.data(), fileSize);
	result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
	return result;
}

bool npc_template_t::isvalid(){
	return !invalid && symbol != "" && description != "" && name != "" && 
		color != "" && speed != "" && abilities != "empty" && hitpoints != ""
		&& attackDamage != "" && rarity > 0 && rarity <= 100;
	
}

bool npc_template_t::isUnique(){
	return abilities.find("UNIQ") <= 0;
}

void npc_template_t::print(){
	cout << name << endl;
   	cout << description << endl;
    	cout << symbol << endl;
    	cout << color << endl;
    	cout << speed << endl;
    	cout << abilities << endl;
    	cout << hitpoints << endl;
    	cout << attackDamage << endl;
	cout << rarity << endl;
}

npc_t npc_template_t::generate(){
	npc_t npc = npc_t();
	npc.name = name;
	npc.description = description;
	npc.speed = dice(speed).roll();
	int intelligent = 0;
    int telepathic = 0;
    int tunnelling = 0;
    int erratic = 0;
	npc.symbol = symbol[0];
	npc.colors = color;
	if(abilities.find("SMART") != string::npos){
		intelligent = INTELLIGENT;
	}
	if(abilities.find("TELE") != string::npos){
		telepathic = TELEPATHIC;
	}
	if(abilities.find("TUNNEL") != string::npos){
		tunnelling = TUNNELLING;
	}
	if(abilities.find("ERRATIC") != string::npos){
		erratic = ERRATIC;
	}
	if(abilities.find("BOSS") != string::npos){
		npc.boss = true;
	}
    npc.character_type = intelligent | telepathic | tunnelling | erratic;
	npc.Damage = dice(attackDamage);
    npc.lastSeenPC = pair_xy_t(0,0);
    npc.dead = false;  
	return npc;
}

bool object_template_t::isvalid(){
	return !invalid 
		&& name != ""
		&& description != ""
		&& type != ""
		&& color != ""
		&& hitbonus != ""
		&& damagebonus != ""
		&& dodgebonus != ""
		&& defensebonus != ""
		&& weight != ""
		&& speedbonus != ""
		&& value != ""
		&& (artifactstatus == "TRUE" || artifactstatus == "FALSE")
		&& rarity >= 0 && rarity <= 100;
}

void object_template_t::print(){
	cout << name << endl;
	cout << description << endl;
	cout << type << endl;
	cout << color << endl;
	cout << hitbonus << endl;
	cout << dodgebonus << endl;
	cout << defensebonus << endl;
	cout << weight << endl;
	cout << speedbonus << endl;
	cout << specialattribute << endl;
	cout << value << endl;
	cout << artifactstatus << endl;
	cout << rarity << endl;
	//cout << "invalid:" << invalid << endl;
}

game_object_t object_template_t::generate(){
	game_object_t obj = game_object_t();
	obj.Name = name;
	obj.Description = description;
	obj.Color = color;
	obj.Damage = dice(damagebonus);
	obj.Defense = dice(defensebonus).roll();
	obj.Speed = dice(speedbonus).roll();
	obj.Hit = dice(hitbonus).roll();
	obj.Value = dice(value).roll();
	obj.Attribute = dice(specialattribute).roll();
	obj.Dodge = dice(dodgebonus).roll();
	obj.Weight = dice(weight).roll();
	obj.Type = type;

	return obj;
}

static npc_template_t parseMonsterTemplate(stringstream &ss){
	npc_template_t mt;
	string line;
	string word;
	while(getline(ss,line)){
		if(line == "END"){
			return mt;
		}

		stringstream ssline(line);
		getline(ssline, word, ' ');
		if(word == "NAME"){
			if(mt.name != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.name = word;
		}
		else if(word == "SYMB"){
			if(mt.symbol != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.symbol = word;
		}
		else if(word == "COLOR"){
			if(mt.color != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.color = word;
		}
		else if(word == "SPEED"){
			if(mt.speed != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.speed = word;
		}
		else if(word == "DAM"){
			if(mt.attackDamage != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.attackDamage = word;
		}
		else if(word == "HP"){
			if(mt.hitpoints != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.hitpoints = word;
		}
		else if(word == "ABIL"){
			if(mt.abilities != "empty"){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.abilities = word;
		}
		else if(word == "DESC"){
			if(mt.description != ""){
				mt.invalid = true;
				return mt;
			}
			string desc;	
			int counter = 1;
			while (getline(ss, word)){
				if(word == "."){
					mt.description = desc;
					break;	
				}
				else{
					if(counter > 1){
					desc += "\n";
					}

				        desc += word;
					counter++;
				}	       
			}
		}
		else if(word == "RRTY"){
			if(mt.rarity > 0){
                   		mt.invalid = true;
                   		return mt;
               	 	}
               	 	else{
				getline(ssline,word);
                    		//try to cast the next value
                    		stringstream convertor;
                    		int number;
                    		convertor << word;
                    		convertor >> number;

                    		if(convertor.fail()){
                       			mt.invalid = true;
                        		return mt;
                    		}
				
				else if(number < 1 || number > 100){
                        		mt.invalid = true;
                        		return mt;
                    		}
				else{
                        		mt.rarity = number;
                    		}
               		}
		}



	}
	return mt;
}	

static object_template_t parseObjectTemplate(stringstream &ss){
	object_template_t mt;
	string line;
	string word;
	while(getline(ss,line)){
		if(line == "END"){
			return mt;
		}

		stringstream ssline(line);
		getline(ssline, word, ' ');
		if(word == "NAME"){
			if(mt.name != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.name = word;
		}
		else if(word == "TYPE"){
			if(mt.type != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.type = word;
		}
		else if(word == "COLOR"){
			if(mt.color != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.color = word;
		}
		else if(word == "SPEED"){
			if(mt.speedbonus != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.speedbonus = word;
		}
		else if(word == "DAM"){
			if(mt.damagebonus != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.damagebonus = word;
		}
		else if(word == "HIT"){
			if(mt.hitbonus != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.hitbonus = word;
		}
		else if(word == "DODGE"){
			if(mt.dodgebonus != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.dodgebonus = word;
		}else if(word == "DEF"){
			if(mt.defensebonus != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.defensebonus = word;
		}
		else if(word == "WEIGHT"){
			if(mt.weight != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.weight = word;
		}
		else if(word == "ATTR"){
			if(mt.specialattribute != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.specialattribute = word;
		}
		else if(word == "VAL"){
			if(mt.value != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.value = word;
		}
		else if(word == "ART"){
			if(mt.artifactstatus != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ssline, word);
			mt.artifactstatus = word;
		}
		else if(word == "DESC"){
			if(mt.description != ""){
				mt.invalid = true;
				return mt;
			}
			string desc;	
			int counter = 1;
			while (getline(ss, word)){
				if(word == "."){
					mt.description = desc;
					break;	
				}
				else{
					if(counter > 1){
					desc += "\n";
					}

				        desc += word;
					counter++;
				}	       
			}
		}
		else if(word == "RRTY"){
			if(mt.rarity > 0){
                   		mt.invalid = true;
                   		return mt;
               	 	}
               	 	else{
				getline(ssline,word);
                    		//try to cast the next value
                    		stringstream convertor;
                    		int number;
                    		convertor << word;
                    		convertor >> number;

                    		if(convertor.fail()){
                       			mt.invalid = true;
                        		return mt;
                    		}
				
				else if(number < 1 || number > 100){
                        		mt.invalid = true;
                        		return mt;
                    		}
				else{
                        		mt.rarity = number;
                    		}
               		}
		}



	}
	return mt;
}	

vector<npc_template_t> parseMonsterTemplates(string filename){

	vector<npc_template_t> templates;
	ifstream in(filename);

	//cannot open the file
	if(!in){
		cout << "Cannot open file\n";
		in.close();
		return templates;
	}


	string file = readFile(filename);
	stringstream ss(file);

	string line;	
	getline(ss,line);
	string line1 = "RLG327 MONSTER DESCRIPTION 1";
	if (line != line1){
		return templates;
	}
	
	//checking for the rest of the descriptors
	while(getline(ss,line)){
		if(line != "BEGIN MONSTER"){
		
		}
		else{

			npc_template_t mt = parseMonsterTemplate(ss);
			//mt.print();
			if(!mt.invalid && mt.isvalid()){
				templates.push_back(mt);
				
			}	
				
		}
	
			


	}

	return templates;
	

}

vector<object_template_t> parseObjectTemplates(string filename){

	vector<object_template_t> templates;
	ifstream in(filename);

	//cannot open the file
	if(!in){
		cout << "Cannot open file\n";
		in.close();
		return templates;
	}


	string file = readFile(filename);
	stringstream ss(file);

	string line;	
	getline(ss,line);
	string line1 = "RLG327 OBJECT DESCRIPTION 1";
	if (line != line1){
		return templates;
	}
	
	//checking for the rest of the descriptors
	while(getline(ss,line)){
		if(line != "BEGIN OBJECT"){
		
		}
		else{

			object_template_t mt = parseObjectTemplate(ss);
			//mt.print();
			if(!mt.invalid && mt.isvalid()){
				templates.push_back(mt);
				
			}	
				
		}
	
			


	}

	return templates;
	

}




