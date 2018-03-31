#ifndef DUNGEON_H
#define DUNGEON_H

#include "heap.h"
#include <string.h>
#include <string>


#define DUNGEON_X				80
#define DUNGEON_Y				21

#define MAX_MONSTER_LIST		21

class Room {
	public:
		int x;
		int y; 
		int w; 
		int h;
	};

typedef int16_t pair_t[2];


enum class terrain_type {
  ter_debug,
  ter_wall,
  ter_wall_immutable,
  ter_floor_room,
  ter_floor_hall,
  ter_floor_downstairs,
  ter_floor_upstairs,
  ter_unknown
};


enum class game_status_t{
	game_status_lost, // == 0
	game_status_won // == 1
};

class dungeon_t {
public:
  int num_rooms;
  Room *rooms;
  terrain_type map[21][80];
  int hardness[21][80];
  int ntmap[21][80];
  int tunmap[21][80];
  heap_t event_heap;
  game_status_t gameStatus = game_status_t::game_status_lost;
  bool lightson = false;
};

class pair_xy_t{
public:
    int x;
    int y;
    pair_xy_t();
    pair_xy_t(int _x, int _y);
    bool isZeros();
};


class game_character_t {
public:
    int character_type = 0;
    pair_xy_t pos;
    int speed = 0;
    bool dead = 0; 
    game_character_t();
    virtual ~game_character_t();

    virtual bool isPC();
    virtual pair_xy_t getNextCell(dungeon_t *d, pair_xy_t pcPos);
    void kill();
};

class pc_t : public game_character_t {
public:
    terrain_type seenmap[21][80];
    pc_t();
    pc_t(dungeon_t *d);
    pc_t(int x, int y);
    virtual bool isPC();
    pair_xy_t getNextCell(dungeon_t *d, pair_xy_t pcPos);
    void updateSeenMap(dungeon_t *d);
    bool canSee(int x, int y);
    void reset(dungeon_t *d);
};

class npc_t: public game_character_t {
public:
    pair_xy_t lastSeenPC;
    int lastSeenDistancMap[21][80];

    npc_t();

    npc_t(dungeon_t *d, pair_xy_t *pts, int numPoints, pair_xy_t pcPos);
    pair_xy_t getNextCell(dungeon_t *d, pair_xy_t pcPos);
    bool isErratic();
    bool isTelepathic();
    bool isIntelligent();
    bool canTunnel();

};



struct game_event_t{
public:
	game_character_t * game_char;
	int turnTime;
};

class players_t{
public:
 pc_t pc;
  npc_t *gameCharacters;
  int num_chars;
  void fillPairArray(pair_xy_t *arr);
};




void killCharacter(game_character_t *gamechar);


pair_xy_t getRandomAdjacentCell(dungeon_t *d, const pair_xy_t source, bool allowWalls);
pair_xy_t getNextStraightLineCellToTarget(dungeon_t *d, pair_xy_t source, pair_xy_t target, bool canTunnel);
pair_xy_t getNextClosestCellToPC(dungeon_t *d, pair_xy_t source, bool canTunnel);
pair_xy_t getNextClosestCellToLastSeenPC(dungeon_t *d, pair_xy_t source, int (*lastSeenDistancMap)[DUNGEON_X], bool canTunnel);
bool hasLineOfSightToPC(dungeon_t *d, pair_xy_t pcPos, pair_xy_t source);
bool isCellOccupied(pair_xy_t *pts, int numPoints,  int16_t yPos, int16_t xPos, pair_xy_t pcPos, bool ignorePC);
pair_xy_t getRandomOpenLocation(dungeon_t *d, pair_xy_t *pts, int numPoints, pair_xy_t pcPos);
pair_xy_t getRandomCell(dungeon_t *d);

game_character_t  *getCharacterFromCell(players_t *pl, uint16_t y, uint16_t x);

void debugprint(std::string message);
void printDungeon(dungeon_t *d, players_t *pl);
void printMonsters(players_t *pl, int startNum);










#endif
