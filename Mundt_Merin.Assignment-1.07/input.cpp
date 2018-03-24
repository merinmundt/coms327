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
using namespace std;

static string readFile(const string &fileName)
{
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return string(bytes.data(), fileSize);
}

static npc_template_t parseMonsterTemplate(stringstream &ss){
	npc_template_t mt;
	string line;
	while(getline(ss,line)){
		if(line == "END"){
			return mt;
		}
		string word;
		getline(ss, word, ' ');
		if(word == "NAME"){
			if(mt.name != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.name = word;
		}
		else if(word == "SYMB"){
			if(mt.symbol != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.symbol = word;
		}
		else if(word == "COLOR"){
			if(mt.color != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.color = word;
		}
		else if(word == "SPEED"){
			if(mt.speed != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.speed = word;
		}
		else if(word == "DAM"){
			if(mt.attackDamage != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.attackDamage = word;
		}
		else if(word == "HP"){
			if(mt.hitpoints != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.hitpoints = word;
		}
		else if(word == "ABIL"){
			if(mt.abilities != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.abilities = word;
		}
		else if(word == "DESC"){
			if(mt.desc != ""){
				mt.invalid = true;
				return mt;
			}
			getline(ss, word);
			mt.color = word;
		}
		else if(word == "RRTY"){
			if(mt.Rarity > 0){
                   		mt.Invalid = true;
                   		return mt;
               	 	}
               	 	else{
                    		//try to cast the next value
                    		stringstream convertor;
                    		int number;
                    		convertor << words[1];
                    		convertor >> number;

                    		if(convertor.fail()){
                       			mt.Invalid = true;
                        		return mt;
                    		}
				
				else if(number < 1 || number > 100){
                        		mt.Invalid = true;
                        		return mt;
                    		}
				else{
                        		mt.Rarity = number;
                    		}
               		}
		}



	}
}	

vector<npc_template_t> parseMonsterTemplates(string filename){

	vector<npc_template_t> templates;
	ifstream in(filename);

	//cannot open the file
	if(!in){
		cout << "Cannot open file\n";
	}
	in.close();

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
			parseMonsterTemplate(ss);	
		}
	
			


	}
	return templates;



	//TODO
	//find beginning of monster 
	//
	//parse through name
	//
	//parse through symbol
	//
	//parse through color
	//
	//parse through desc
	//
	//parse through speed
	//
	//parse through attack damage
	// 
	//parse through hitpoints
	//
	//parse through rarity
	//
	//parse through abilities
	//
	//find end of monster
	//
	//look for more monsters

}





