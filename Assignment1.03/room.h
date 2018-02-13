#include <stdint.h>

typedef	struct room {
	
		u_int8_t x;
		u_int8_t y; 
		u_int8_t w; 
		u_int8_t h;
	}Room;

typedef int16_t pair_t[2];

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_debug,
  ter_wall,
  ter_wall_immutable,
  ter_floor,
  ter_floor_room,
  ter_floor_hall,
  ter_player,
} terrain_type_t;

typedef struct dungeon {
  uint32_t num_rooms;
  Room *rooms;
  terrain_type_t map[21][80];
  /* Since hardness is usually not used, it would be expensive to pull it *
   * into cache every time we need a map cell, so we store it in a        *
   * parallel array, rather than using a structure to represent the       *
   * cells.  We may want a cell structure later, but from a performanace  *
   * perspective, it would be a bad idea to ever have the map be part of  *
   * that structure.  Pathfinding will require efficient use of the map,  *
   * and pulling in unnecessary data with each map cell would add a lot   *
   * of overhead to the memory system.                                    */
  uint8_t hardness[21][80];
  pair_t pc; //player character
  uint32_t ntmap[21][80];
  uint32_t tunmap[21][80];
} dungeon_t;