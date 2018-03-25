#include "input.h"
#include <vector>
#include <iostream>

//TODO in main: parse monster file
//
//
//
//
//

int main(int argc, char *argv[]){
	vector<npc_template_t> monsters = parseMonsterTemplates("file.txt");
	if(monsters == NULL || monsters.size() == 0){
		cout << "No monsters available for attacking" << endl;
	}
	
	else{
		for(auto &t : monsters){
			t.print();
			cout << "\n";
		}
	}
}	
