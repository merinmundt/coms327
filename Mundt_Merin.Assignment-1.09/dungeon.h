#ifndef DUNGEON_H
#define DUNGEON_H

#include "heap.h"
#include <string.h>
#include <string>
#include <vector>
#include "dice.h"
#include <map>

using namespace std;

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

class game_object_t{
public:
    string Name;
    string Description;
    string Type;
    string Color;
    int Hit;
    dice Damage;
    int Dodge;
    int Defense;
    int Weight;
    int Speed;
    int Attribute;
    int Value;
    string Artifact;
    int Rarity;
    pair_xy_t pos;
    int getColors();
    

};

class game_character_t {
public:
    int character_type = 0;
    pair_xy_t pos;
    int speed = 10;
    bool dead = false; 
    bool boss = false;
    game_character_t();
    virtual ~game_character_t();
    string name;
    string description;
    int Hit = 100;
    dice Damage = dice("0+1d4");
    char symbol;
    string colors;
    int getColors();

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
    void attack(game_character_t *gc);
    vector <game_object_t> carrySlots;
    map <string,game_object_t> equiptmentSlots;

};

class npc_t: public game_character_t {
public:
    pair_xy_t lastSeenPC;
    int lastSeenDistancMap[21][80];

   
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
  vector<string> deadMonsterNames;
  vector<string> placedMonsterNames;
  vector<string> placedObjectNames;
  vector<string> destroyedObjectNames;
  vector<game_object_t> gameObjects;
};






void killCharacter(game_character_t *gamechar);


pair_xy_t getRandomAdjacentCell(dungeon_t *d, const pair_xy_t source, bool allowWalls);
pair_xy_t getNextStraightLineCellToTarget(dungeon_t *d, pair_xy_t source, pair_xy_t target, bool canTunnel);
pair_xy_t getNextClosestCellToPC(dungeon_t *d, pair_xy_t source, bool canTunnel);
pair_xy_t getNextClosestCellToLastSeenPC(dungeon_t *d, pair_xy_t source, int (*lastSeenDistancMap)[DUNGEON_X], bool canTunnel);
bool hasLineOfSightToPC(dungeon_t *d, pair_xy_t pcPos, pair_xy_t source);
bool isCellOccupied(pair_xy_t *pts, int numPoints,  int16_t yPos, int16_t xPos, pair_xy_t pcPos, bool ignorePC); 
bool doesCellHaveObject(players_t *pl, int x, int y);
pair_xy_t getRandomOpenLocation(dungeon_t *d, pair_xy_t *pts, int numPoints, pair_xy_t pcPos);
pair_xy_t getRandomCell(dungeon_t *d);
pair_xy_t getRandomLocation(dungeon_t *d);

game_object_t popObjectFromCell(players_t *pl, int x, int y);
game_object_t *getObjectfromCell(players_t *pl, int y, int x);
game_character_t  *getCharacterFromCell(players_t *pl, uint16_t y, uint16_t x);

void wearObject(pc_t &pc, size_t slot);
void showMonster(game_character_t *npc);
void showObject(pc_t &pc, size_t slot);
void dropObject(players_t *pl, size_t slot);
void expungeObject(pc_t &pc, size_t slot);
void takeOffEquipment(players_t *pl, string equipmentName);
void listPcEquipment(pc_t &pc);
void listPcInventory(pc_t &pc);
void debugprint(std::string message);
void printDungeon(dungeon_t *d, players_t *pl);
void printMonsters(players_t *pl, int startNum);
size_t promptforCarrySlot(pc_t & pc);
string promptForEquipmentName(pc_t &pc);












#endif
