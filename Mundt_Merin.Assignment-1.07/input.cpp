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

bool npc_template_t::isvalid(){
	return !invalid && symbol != "" && description != "" && name != "" && 
		color != "" && speed != "" && abilities != "empty" && hitpoints != ""
		&& attackDamage != "" && rarity > 0 && rarity <= 100;
	
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





