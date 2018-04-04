#include "dungeon.h"
#include <stdlib.h>
#include <memory>
#include "macros.h"
#include <time.h>
#include <ncurses.h>
#include <sstream>


#define INTELLIGENT				1
#define TELEPATHIC				2
#define TUNNELLING				4
#define ERRATIC					8


pair_xy_t::pair_xy_t(int _x, int _y){
      x = _x;
      y = _y;
  }
bool pair_xy_t::isZeros(){
      return x == 0 && y == 0;
  }

pair_xy_t::pair_xy_t(){

  }




game_character_t::game_character_t(){
    pos.x = 1;
    pos.y = 1;
    dead = false;
  }

game_character_t::~game_character_t(){

  }

pair_xy_t game_character_t::getNextCell(dungeon_t *d, pair_xy_t pcPos){
        return pair_xy_t(0,0); //todo implement
    }

bool game_character_t::isPC(){
      return false;
  }

void game_character_t::kill(){
      dead = true;
  }


    pc_t::pc_t():game_character_t(){

    }

    pc_t::pc_t(dungeon_t *d){
        //random location
        int done = 0;
        while(done == 0){
            int yRand = rand() % 19 + 1;
            int xRand = rand() % 78 + 1;
            if(mapxy(xRand, yRand) == terrain_type::ter_floor_room){
                pos.x= xRand;
                pos.y = yRand;
                speed = 10;
                dead = false;
                done = 1;
            }
        }
    }
    int game_object_t::getColors(){
        stringstream ss(Color);
        string color;
        getline(ss,color, ' ');
        if(color == "RED"){
            return COLOR_RED;
        }
        else if(color == "BLUE"){
            return COLOR_BLUE;
        }
        else if(color == "GREEN"){
            return COLOR_GREEN;
        }
        else if(color == "CYAN"){
            return COLOR_CYAN;
        }
        else if(color == "YELLOW"){
            return COLOR_YELLOW;
        }
        else if(color == "MAGENTA"){
            return COLOR_MAGENTA;
        }
        else{
            return COLOR_WHITE;
        }
    }

    int game_character_t::getColors(){
        stringstream ss(colors);
        string color;
        getline(ss,color, ' ');
        if(color == "RED"){
            return COLOR_RED;
        }
        else if(color == "BLUE"){
            return COLOR_BLUE;
        }
        else if(color == "GREEN"){
            return COLOR_GREEN;
        }
        else if(color == "CYAN"){
            return COLOR_CYAN;
        }
        else if(color == "YELLOW"){
            return COLOR_YELLOW;
        }
        else if(color == "MAGENTA"){
            return COLOR_MAGENTA;
        }
        else{
            return COLOR_WHITE;
        }
    }

    pc_t::pc_t(int x, int y){
        pos.x = x;
        pos.y = y;
        speed = 10;
        dead = false;
    }

    bool pc_t::isPC(){
        return true;
    }

    pair_xy_t pc_t::getNextCell(dungeon_t *d, pair_xy_t pcPos){
        return pair_xy_t(0,0); //todo implement
    }

    void pc_t::updateSeenMap(dungeon_t *d){
        //update plus or minus 2 all the way around
        for(int y = pos.y - 2;y <= pos.y + 2;y++){
            for(int x = pos.x - 2;x <= pos.x + 2;x++){
                if(x >= 0 && y >= 0 && x < DUNGEON_X && y < DUNGEON_Y){
                    seenmap[y][x] = mapxy(x,y);
                }
            }
        }
    }

    void pc_t::reset(dungeon_t *d){
        //update plus or minus 2 all the way around
        for(int y = 0;y < DUNGEON_Y;y++){
            for(int x = 0;x < DUNGEON_X;x++){
                seenmap[y][x] = terrain_type::ter_unknown;
            }
        }
        updateSeenMap(d);
    }

    bool pc_t::canSee(int x, int y){
        return abs(x - pos.x) <= 2 && abs(y - pos.y) <= 2;
    }


    bool npc_t::isErratic() {
        return character_type & ERRATIC;
    }

    bool npc_t::canTunnel(){
        return character_type & TUNNELLING;
    }

    bool npc_t::isIntelligent(){
        return character_type & INTELLIGENT;
    }

    bool npc_t:: isTelepathic(){
        return character_type & TELEPATHIC;
    }


    pair_xy_t npc_t::getNextCell(dungeon_t *d, pair_xy_t pcPos){
        pair_xy_t newPos = pair_xy_t(0,0);
        if(isErratic() && rand() % 2 == 0){
			newPos = getRandomAdjacentCell(d, pos, canTunnel());
			
		}
		else{
			//if we are telepathic we always know where the PC is.
			//if not we only know if we have line of sight or we go with 
			//the last time we had line of sight
			if(isTelepathic()){
				if(isIntelligent()){
					newPos = getNextClosestCellToPC(d, pos, canTunnel());
				}else{
					newPos = getNextStraightLineCellToTarget(d, pos, pcPos, canTunnel());
				}
			}else{
				if(hasLineOfSightToPC(d, pcPos, pos)){
					if(isIntelligent()){
						newPos = getNextClosestCellToPC(d, pos, canTunnel());
					}else{
						newPos = getNextStraightLineCellToTarget(d, pos, pcPos, canTunnel());
					}
				}else{
					if(lastSeenPC.isZeros()){
						//has no last seen PC
						newPos = getRandomAdjacentCell(d, pos, canTunnel());
					}else{
						//has a location to try and get to 
						if(isIntelligent()){
							newPos = getNextClosestCellToLastSeenPC(d,pos, lastSeenDistancMap, canTunnel());
						}else{
							newPos = getNextStraightLineCellToTarget(d, pos, lastSeenPC, canTunnel());
                        }
                    }
                }
            }
		}

        return newPos;
    }

game_object_t *getObjectfromCell(players_t *pl, int y, int x){
   
    for(game_object_t &g : pl->gameObjects){
        if((y == g.pos.y) && (x == g.pos.x)){
            return &g;
        }
    
    }

    return NULL;
}

game_character_t  *getCharacterFromCell(players_t *pl, uint16_t y, uint16_t x){
    game_character_t *gc;
    gc = NULL;

    for(int i = 0;i < pl->num_chars;i++){
        if(isSameCell(pl->gameCharacters[i].pos, x, y)){
            gc = &pl->gameCharacters[i];
        }
    }

    if(gc == NULL){
        if(pl->pc.pos.x == x && pl->pc.pos.y == y){
            return &pl->pc;
        }
    }

    return gc;
}
