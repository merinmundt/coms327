#include <stdint.h>
#include "heap.h"

typedef	struct room {
	
		u_int8_t x;
		u_int8_t y; 
		u_int8_t w; 
		u_int8_t h;
	}Room;

typedef int16_t pair_t[2];

typedef struct pairxy{
  uint16_t x;
  uint16_t y;
} pair_xy_t;

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_debug,
  ter_wall,
  ter_wall_immutable,
  ter_floor_room,
  ter_floor_hall
} terrain_type_t;


typedef struct GameCharacter {
  uint8_t character_type;
  pair_xy_t pos;
  uint8_t speed;
  pair_xy_t lastSeenPC;
  uint32_t lastSeenDistancMap[21][80];
  uint8_t isPC;
  uint8_t dead;
} game_character_t;


typedef struct gameEvent{
	game_character_t * game_char;
	uint32_t turnTime;
} game_event_t;


typedef enum gameStatus{
	game_status_lost, // == 0
	game_status_won // == 1
} game_status_t;

typedef struct dungeon {
  uint32_t num_rooms;
  Room *rooms;
  terrain_type_t map[21][80];
  uint8_t hardness[21][80];
  game_character_t pc; //player character
  uint32_t ntmap[21][80];
  uint32_t tunmap[21][80];
  game_character_t *gameCharacters;
  uint32_t num_chars;
  heap_t event_heap;
  game_status_t gameStatus;
} dungeon_t;
