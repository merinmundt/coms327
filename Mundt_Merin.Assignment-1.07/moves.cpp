#include "dungeon.h"
#include "macros.h"
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <ncurses.h>

pair_xy_t getRandomAdjacentCell(dungeon_t *d, pair_xy_t source, bool allowWalls){
	//try randomly adding 0, 1, or -1 to y and x (but not 0 and 0)
	//printf("get randmon adjacent\n");
	pair_xy_t p = pair_xy_t(0,0);
	int xcounter = 0;
	int yPos = -1;
	int xPos = -1;
	int foundCell = 0;
	while(foundCell == 0 && xcounter < 1000){
		yPos = rand() % 3 - 1;
		xPos = rand() % 3 - 1;
		if(yPos == 0 && xPos == 0)
			continue;
		if(mapxy(source.x + xPos, source.y + yPos) != terrain_type::ter_wall_immutable 
				&& (mapxy(source.x + xPos, source.y + yPos) != terrain_type::ter_wall || allowWalls)){
			p = pair_xy_t(source.x + xPos, source.y + yPos);
			break;
		}
		xcounter++;
	}

	return p;
}



pair_xy_t getNextStraightLineCellToTarget(dungeon_t *d, pair_xy_t source, pair_xy_t target, bool canTunnel){
	//printf("get straightline to target\n");
	//try to move along x first then y
	if(target.y == source.y && target.x == source.x)
		return source;

	if(target.x < source.x && (canTunnel || hardnessxy(source.x - 1, source.y) == 0)){
		return pair_xy_t(source.x - 1, source.y);
    }

	if(target.x > source.x && (canTunnel || hardnessxy(source.x + 1, source.y) == 0)){
		return pair_xy_t(source.x + 1, source.y);
	}

	if(target.y > source.y && (canTunnel || hardnessxy(source.x, source.y + 1) == 0)){
		return pair_xy_t(source.x, source.y + 1);
	}

	if(target.y < source.y && (canTunnel || hardnessxy(source.x, source.y - 1) == 0)){
		return pair_xy_t(source.x, source.y - 1);
	}

	return source;
}


pair_xy_t getNextClosestCellToPC(dungeon_t *d, pair_xy_t source, bool canTunnel){
	//printf("get next closest cell\n");
	//use the distance maps
	pair_xy_t nextCell = pair_xy_t(0,0);
	int lastDistance = INT32_MAX;
	int newX, newY;
	for(int yOffset = -1;yOffset < 2;yOffset++){
		for(int xOffset = -1;xOffset < 2;xOffset++){
			newX = source.x + xOffset;
			newY = source.y + yOffset;
			if(newX >= DUNGEON_X || newX <= 0 || newY >= DUNGEON_Y || newY <= 0)
				continue;
			if(mapxy(newX, newY) != terrain_type::ter_wall_immutable){
				if(canTunnel){
					if(tunmapxy(newX, newY) < lastDistance){
						lastDistance = tunmapxy(newX, newY);
						nextCell = pair_xy_t(newX, newY);
					}
					
				}else{
					if(hardnessxy(newX, newY) == 0){
						if(ntmapxy(newX, newY) < lastDistance){
							lastDistance = ntmapxy(newX, newY);
							nextCell = pair_xy_t(newX, newY);
						}
					}
				}
			}
			
		}
	}
	return nextCell;
}

pair_xy_t getNextClosestCellToLastSeenPC(dungeon_t *d, pair_xy_t source, int (*lastSeenDistancMap)[DUNGEON_X], bool canTunnel){
	//printf("get next closest cell to target\n");
	pair_xy_t nextCell = pair_xy_t(0,0);
	int lastDistance = INT32_MAX;
	int newX, newY;
	for(int yOffset = -1;yOffset < 2;yOffset++){
		for(int xOffset = -1;xOffset < 2;xOffset++){
			newX = source.x + xOffset;
			newY = source.y + yOffset;
			if(newX >= DUNGEON_X || newX <= 0 || newY >= DUNGEON_Y || newY <= 0)
				continue;
			if(mapxy(newX, newY) != terrain_type::ter_wall_immutable){
				if(canTunnel){
					if(lastSeenDistancMap[newY][newX] < lastDistance){
						lastDistance = tunmapxy(newX, newY);
						nextCell = pair_xy_t(newX, newY);
					}
					
				}else{
					if(hardnessxy(newX, newY) == 0){
						if(lastSeenDistancMap[newY][newX] < lastDistance){
							lastDistance = ntmapxy(newX, newY);
							nextCell = pair_xy_t(newX, newY);
						}
					}
				}
			}
			
		}
	}
	return nextCell;
}

bool hasLineOfSightToPC(dungeon_t *d, pair_xy_t pcPos, pair_xy_t source){
	//if pc and source have same x or y value and nothing but floor and cooridor is between them
	int hasSight = 1;
	int y,x;
	if(source.x ==  pcPos.x){
		if(source.y > pcPos.y){
			for(y = source.y; y > pcPos.y; y--){
				if(mapxy(source.x, y) != terrain_type::ter_floor_hall && mapxy(source.x, y) != terrain_type::ter_floor_room){
					hasSight = 0;
					break;
				}
			}
			if(hasSight == 1)
				return 1;
		}else{
			hasSight = 1;
			for(y = source.y; y < pcPos.y; y++){
				if(mapxy(source.x, y) != terrain_type::ter_floor_hall && mapxy(source.x, y) != terrain_type::ter_floor_room){
					hasSight = 0;
					break;
				}
			}
			if(hasSight == 1)
				return 1;
		}
	} else if(source.y ==  pcPos.y){
		if(source.x >pcPos.x){
			for(x = source.x; x > pcPos.x; x--){
				if(mapxy(x, source.y) != terrain_type::ter_floor_hall && mapxy(x, source.y) != terrain_type::ter_floor_room){
					hasSight = 0;
					break;
				}
			}
			if(hasSight == 1)
				return 1;
		}else{
			hasSight = 1;
			for(x = source.x; x < pcPos.x; x++){
				if(mapxy(x, source.y) != terrain_type::ter_floor_hall && mapxy(x, source.y) != terrain_type::ter_floor_room){
					hasSight = 0;
					break;
				}
			}
			if(hasSight == 1)
				return 1;
		}
	}

	return 0;
}

bool isCellOccupied(pair_xy_t *pts, int numPoints,  int16_t yPos, int16_t xPos, pair_xy_t pcPos, bool ignorePC){
	if(!ignorePC){
		if(pcPos.y== yPos && pcPos.x == xPos){
			return true;
		}
	}
	
	
	for(int i = 0;i < numPoints;i++){
		if(pts[i].y == yPos && pts[i].x== xPos){
			return true;
		}
	}
	//if we got here, the cell is unOccupied;
	return false;
}

pair_xy_t getRandomOpenLocation(dungeon_t *d, pair_xy_t *pts, int numPoints, pair_xy_t pcPos){
	//cycle through random locations, if empty and not rock, return it
	int yPos = -1;
	int xPos = -1;
	while(1 == 1){
		xPos = rand() % DUNGEON_X;
		yPos = rand() % DUNGEON_Y;
		
		if(mapxy(xPos, yPos) == terrain_type::ter_floor_hall || mapxy(xPos, yPos) == terrain_type::ter_floor_room){
			//its a floor
			if(!isCellOccupied(pts, numPoints, yPos, xPos,pcPos, false)){
				return pair_xy_t(xPos, yPos);
			}
		}
	}
}

pair_xy_t getRandomCell(dungeon_t *d){
	int xPos = rand() % (DUNGEON_X - 2) + 1;
	int yPos = rand() % (DUNGEON_Y - 2) + 1;
	return pair_xy_t(xPos, yPos);
}

