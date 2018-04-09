#ifndef OUTPUT_H
#define OUTPUT_H

#include "dungeon.h"
#include "macros.h"
#include <ncurses.h>
#include <string.h>
#include <string>
#include <map>



#define ROOMFLOOR '.'
#define COORIDORFLOOR '#'
#define PCFLOOR '@'

static char hexchars[16] = {'0','1','2','3','4','5','6','7','8', '9', 'a', 'b','c','d','e','f'};
#define itohex(i) (hexchars[i])

static map<string, char> symbolMap = {
	{"WEAPON", '|'},
	{"OFFHAND", ')'},
	{"RANGED", '}'},
	{"ARMOR", '['},
	{"HELMET", ']'},
	{"CLOAK", '('},
	{"GLOVES", '{'},
	{"BOOTS", '\\'},
	{"RING", '='},
	{"AMULET", '\"'},
	{"LIGHT", '_'},
	{"SCROLL", '~'},
	{"BOOK", '?'},
	{"FLASK", '!'},
	{"GOLD", '$'},
	{"AMMUNITION", '/'},
	{"FOOD", ','},
	{"WAND", '-'},
	{"CONTAINER", '%'},
	{"STACK", '&'}
};

size_t promptforCarrySlot(pc_t & pc){
	clear();
	move(0,0);
	printw("Please enter a carry slot:\n");
	for(size_t i = 0; i < 10; i++){
		printw("%d: ", i);
		if(pc.carrySlots.size() >= i + 1){
			printw("%s-%s\n", pc.carrySlots[i].Name.c_str(), pc.carrySlots[i].Type.c_str());
		}
		else{
			printw("Empty\n");
		}
	}
	refresh();
	int result = getch() - 48;
	clear();
	return result;

}

void listPcInventory(pc_t &pc){
	clear();
	refresh();
	move(0,0);
	printw("PC Inventory:");
	int i = 0;
	for(game_object_t &gobj : pc.carrySlots){
		move(i + 1,0);
		printw("%d: %s - %s", i, gobj.Type.c_str(), gobj.Name.c_str());
		i++;
	}
	refresh();
	getch();
	clear();
}

void listPcEquipment(pc_t &pc){
	clear();
	refresh();
	move(0,0);
	printw("PC Equipment:");
	map<string, game_object_t>::iterator it;
	int i =  0;
	for(it = pc.equiptmentSlots.begin(), i = 0;it != pc.equiptmentSlots.end();it++, i++){
		game_object_t * gobj = &it->second;
		move(i + 1,0);
		printw("%c: %s - %s\n", 97 + i, it->first.c_str(), gobj->Name.c_str()  );
	}
	refresh();
	getch();
	clear();
}

static string equipnames[12] = {"WEAPON", "OFFHAND", "RANGED", "ARMOR", "HELMET", "CLOAK", "GLOVES", "BOOTS", "AMULET", "LIGHT", "RING1", "RING2"};

string promptForEquipmentName(pc_t &pc){
    clear();
    move(0,0);
    printw("Pick a piece of Equipment");
    for(int i = 0;i< 12;i++){
        move(i + 1,0);
        printw("%c: ", i + 97);
        if(pc.equiptmentSlots.count(equipnames[i]) > 0){
            printw("%s - %s", pc.equiptmentSlots[equipnames[i]].Type.c_str(), pc.equiptmentSlots[equipnames[i]].Name.c_str());
        }else{
			printw("<EMPTY>");
		}
    }
	int result = getch();
	if(result - 97 >= 0 && result - 97 < 12){
		return equipnames[result - 97];
	}else
	return "";
}

void showMonster(game_character_t *npc){
	clear();
	move(0,0);
	printw("Name: %s\nDescription: %s\nHitpoints: %d\nDead:%d\n",
		npc->name.c_str(), npc->description.c_str(), npc->Hit, npc->dead);
	refresh();
	getch();
	clear();
}

void showObject(pc_t &pc, size_t slot){
	clear();
	move(0,0);
	if(slot < pc.carrySlots.size()){
		printw("ITEM: %s\nName: %s\nDescription: %s\n",
			pc.carrySlots[slot].Type.c_str(),
			pc.carrySlots[slot].Name.c_str(),
			pc.carrySlots[slot].Description.c_str());
	}else{
		printw("Invalid or empty carry slot");
	}
	refresh();
	getch();
	clear();
}

void printDungeon(dungeon_t *d, players_t *pl){
	//printf("Dungeon\n");
	refresh();
	//move(1,0);
	for (int y = 0; y < DUNGEON_Y; y++) {
		move(y + 1,0);
		for (int x = 0; x < DUNGEON_X; x++) {
			if(isSameCell(pl->pc.pos, x, y) && !pl->pc.dead){
				addch(PCFLOOR);
				continue;
			}
			
			terrain_type terrain = d->lightson ? mapxy(x,y) : seenmapxy(x,y);
			game_character_t *gamechar = getCharacterFromCell(pl, y, x);
			if(gamechar && !gamechar->dead && (pl->pc.canSee(x, y) || d->lightson)){
				attron(COLOR_PAIR(gamechar->getColors()));
				addch((gamechar->symbol));
				attroff(COLOR_PAIR(gamechar->getColors()));
				continue;
			}

			game_object_t *gameobject = getObjectfromCell(pl, y, x);
			if(gameobject && (pl->pc.canSee(x, y) || d->lightson)){
				attron(COLOR_PAIR(gameobject->getColors()));
				addch((symbolMap[gameobject->Type]));
				attroff(COLOR_PAIR(gameobject->getColors()));
				
			}

			else{
				switch (terrain) {
					case terrain_type::ter_wall:
						addch(' ');
						break;
					case terrain_type::ter_wall_immutable:
						if(y == 0 || y == DUNGEON_Y - 1)
							addch('-');
						else if(x == 0 || x == DUNGEON_X - 1)
							addch('|');
						else
							addch(' ');
						break;
					case terrain_type::ter_floor_room:
						addch(ROOMFLOOR);
						break;
					case terrain_type::ter_floor_hall:
						addch(COORIDORFLOOR);
						break;				
					case terrain_type::ter_floor_downstairs:
						addch('>');
						break;
					case terrain_type::ter_floor_upstairs:
						addch('<');
						break;	
					case terrain_type::ter_unknown:
						addch(' ');
						break;	
					default:
						addch('*');
						//fprintf(stderr, "Debug character at %d, %d\n", x, y);
						break;
						
				}
			}
    	}
    	//putchar('\n');
	}
	//printw("%d  %d %d  %d", mapxy(0,0), mapxy(0,1), mapxy(2,0), mapxy(78,0));
	refresh();
    
}

void printMonsters(players_t *pl, int startNum){
	clear();
	refresh();
	move(0,0);
	int line = 1;
	int endNum = MAX_MONSTER_LIST < pl->num_chars - startNum	
		? startNum + MAX_MONSTER_LIST - 1
		: pl->num_chars - 1;
	for(int i = startNum;i <= endNum;i++){
		move(line,0);
		int diffY = pl->gameCharacters[i].pos.y - pl->pc.pos.y;
		int diffX = pl->gameCharacters[i].pos.x - pl->pc.pos.x; 
		printw("%c, %d %s %d %s Dead:%s pos %d, %d speed %d",
			itohex(pl->gameCharacters[i].character_type),
			abs(diffY),
			diffY >= 0 ? "north" : "south",
			abs(diffX),
			diffX >= 0 ? "east" : "west",
			pl->gameCharacters[i].dead ? "Yes" : "No",
			pl->gameCharacters[i].pos.x,
			pl->gameCharacters[i].pos.y,
			pl->gameCharacters[i].speed
			);
		line++;
		refresh();
	}
	move(line,0);
	printw("pc pos %d, %d", pl->pc.pos.x, pl->pc.pos.y);
	refresh();
}

void debugprint(std::string message){
    move(0,0);
    
    printw("%s", message.c_str());
}

#endif
