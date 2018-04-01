#ifndef OUTPUT_H
#define OUTPUT_H

#include "dungeon.h"
#include "macros.h"
#include <ncurses.h>
#include <string.h>
#include <string>



#define ROOMFLOOR '.'
#define COORIDORFLOOR '#'
#define PCFLOOR '@'

static char hexchars[16] = {'0','1','2','3','4','5','6','7','8', '9', 'a', 'b','c','d','e','f'};
#define itohex(i) (hexchars[i])

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
