#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <ncurses.h>
#include "heap.h"
#include "macros.h"
#include "dungeon.h"
#include "input.h"
#include <vector>
#include <algorithm>

using namespace std;



typedef struct corridor_path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} corridor_path_t;

typedef enum dim {
  dim_x,
  dim_y,
  num_dims
} dim_t;


/* options descriptor  for --save and --load*/
static struct option longopts[] = {
        { "save", no_argument, NULL, 's' },
        { "load", no_argument, NULL,'l'},
		{ "y", required_argument, NULL,'y'},
		{ "x", required_argument, NULL,'x'},
		{ "nummon", required_argument, NULL, 'n'}
};

//unsigned char dungeon[21][80];
//unsigned char hardness[21][80];
//unsigned char distMap[21][80];
//unsigned char distSimpleMap[21][80];



int checkRooms(dungeon_t *d);
void saveDungeon(dungeon_t *d);
void loadDungeon(dungeon_t *d);
std::string getGameDirectory();
std::string getGameFilename();
void printHardness();

static int quit = 0;
static int yPC = 0;
static int xPC = 0;
static int numMonsters = 10;
static game_event_t gevents[100000];
static int eventCounter = 0;
vector<npc_template_t> Monsters;
vector<object_template_t> Objects;



static void makePlayerCharacter(dungeon_t *d, players_t *pl){
	pair_xy_t pos = pair_xy_t(xPC, yPC);
	if(pos.y > 0 && pos.y < DUNGEON_Y - 1 && pos.x > 0 && pos.x < DUNGEON_X - 1){
		if(mapxy(pos.x, pos.y) == terrain_type::ter_floor_room){
			//go ahead place the character
			pl->pc = pc_t(pos.x, pos.y);
		}else{
			pl->pc = pc_t(d);
		}
	}else{
		pl->pc = pc_t(d);
	}

	pl->pc.reset(d);
}


void makeBorder(dungeon_t *d){
	for(int y = 0;y < DUNGEON_Y;y++){
		for(int x = 0;x < DUNGEON_X;x++){
			if(y == 0 || y == DUNGEON_Y -1 || x == 0 || x == DUNGEON_X - 1){
				mapxy(x, y) = terrain_type::ter_wall_immutable;
				hardnessxy(x,y) = 255;
			}else{
				mapxy(x,y) = terrain_type::ter_wall;
				hardnessxy(x,y) =  rand() % 254 + 1; //random hardness from 1 to 254
			}
		}
	}
}

static Room makeRoom(dungeon_t *d, int w, int h){
		//pick a random spot in the grid and see if the room fits
	//and does not overlap any rooms or are within 1 cell of another room
	int posx = rand() % 76 + 2; //2 to 77
	int posy = rand() % 17 + 2; //2 to 18
	//printf("Attempting room of %d x %d at %d,%d\n", w, h, posx, posy);
	if(posx + w <= DUNGEON_X - 2 && posy + h <= DUNGEON_Y - 2){
		//room fits.  now check for overlap plus one on each side
		//printf("room Fits\n");
		for(int y = posy - 1;y < posy + h + 1;y++){
			for(int x = posx - 1;x < posx + w + 1;x++){
				//printf("cell at %d, %d, is \"%c\"\n",j, i, dungeon[i][j]);
				if(mapxy(x, y) == terrain_type::ter_wall){
					//all good
				}
				else{
					return (Room){0,0,0,0};
				}
			}

		}
		//printf("Room All Clear\n");
		//printf("Room Made\n");
		return (Room){posx, posy,w, h};
	}

	return (Room) {0,0,0,0};
	
}


static void fillRoom(dungeon_t *d, Room r){
	//fill room with .'s
	//printf("Filling room of %d x %d\n", r.w, r.h);
	for(int y = r.y;y < r.y + r.h;y++){
		for(int x = r.x;x < r.x + r.w;x++){
			//printf("Filling %d, %d with terrain_type::ter_floor_room\n",x, y);
			mapxy(x,y) = terrain_type::ter_floor_room;
			hardnessxy(x,y) = 0;
		}

	}
}

static void fillRooms(dungeon_t *d){
	for(int i = 0;i < d->num_rooms;i++){
		fillRoom(d, d->rooms[i]);
	}
}


static void makeCoorridor(dungeon_t *d, Room a, Room b){
	//printf("Making Cooridor from room {(%d,%d) of %d x %d} to room {(%d,%d) of %d x %d}\n", a.x, a.y, a.w, a.h, b.x, b.y, b.w, b.h);
	
	if (a.x > b.x){

		for(int x = a.x; x>b.x; x--){
			if (mapxy(x, a.y) != terrain_type::ter_floor_room){
				mapxy(x, a.y) = terrain_type::ter_floor_hall;
				hardnessxy(x, a.y) = 0;
			}
		}
	}

	else{

		for(int x = a.x; x<b.x; x++){
			if (mapxy(x, a.y) != terrain_type::ter_floor_room){
				mapxy(x, a.y) = terrain_type::ter_floor_hall;
				hardnessxy(x, a.y) = 0;
			}
		}

	}

	if (a.y > b.y){

		for(int y = a.y; y>b.y; y--){
			if (mapxy(b.x, y) != terrain_type::ter_floor_room){
     			mapxy(b.x, y) = terrain_type::ter_floor_hall;
				hardnessxy(b.x, y) = 0;
			}	
		}
	}

	else{
		for(int y = a.y; y<b.y; y++){
     			if (mapxy(b.x, y) != terrain_type::ter_floor_room){
	      			mapxy(b.x, y) = terrain_type::ter_floor_hall;
					hardnessxy(b.x, y)= 0;
				}
		}

	}


}


static void makeCoorridors(dungeon_t *d){
	for(int i = 0;i < d->num_rooms - 1; i++){
		makeCoorridor(d, d->rooms[i], d->rooms[i + 1]);
	}
}

static void makeRooms(dungeon_t *d, int numRooms){
	//printf("Attempting to make %d rooms\n", numRooms);
	Room rooms[numRooms];
	//intialize all rooms with zeros
	/*for(int i = 0;i < numRooms; i++){
		Room rm = {0,0,0,0};
		rooms[i] = rm;
	}*/
	for(int i = 0;i < numRooms; i++){
		//printf("Room number %d\n",i);
		if(checkRooms(d) == 0){
			//printf("dungeon full\n");
			return;
		}
		
		//make a new room of at least 3 x 2 up to 10 x 10
		int roomH = rand() % 9 + 2;
		int roomW = rand() % 8 + 3;
		//printf("new dimensions %d x %d\n", roomH, roomW);
		int attempts = 1;
		///try 2000 times to make a room
		Room createdRoom;
		//printf("createdRoom 2nd pass %d\n", createdRoom.w);
		while(attempts < 2000 && (createdRoom.w == 0 || attempts == 1)){
			createdRoom = makeRoom(d, roomW, roomH);
			attempts++;
		}

		rooms[i] = createdRoom;
		

	}

	//remove zeroed rooms
	//count non zeroed rooms
	
	int actualRoomsNum = 0;
	for (int i = 0; i < numRooms; i++){
		if(rooms[i].w > 0){
			actualRoomsNum++;
		}
	}
	//create new rooms array with filled rooms only
	Room actualRooms[actualRoomsNum];
	for (int i = 0; i < numRooms; i++){
		if(rooms[i].w > 0){
			actualRooms[i] = rooms[i];
		}
	}

	d->num_rooms = actualRoomsNum;
	d->rooms = new Room[actualRoomsNum];
	for(int i = 0;i < d->num_rooms;i++){
		d->rooms[i].x = actualRooms[i].x;
		d->rooms[i].y = actualRooms[i].y;
		d->rooms[i].w = actualRooms[i].w;
		d->rooms[i].h = actualRooms[i].h;
	}
	fillRooms(d);
	makeCoorridors(d);
	
}

static void makeStairs(dungeon_t *d){
	//find random room or cooridor locations
	int stairCount = 0;
	int y, x;
	while(stairCount < 4){
		y = rand() % (DUNGEON_Y - 1) + 1; //1 to 20
		x = rand() % (DUNGEON_X - 1) + 1; //1 to 78
		if(mapxy(x,y) == terrain_type::ter_floor_hall || mapxy(x,y) == terrain_type::ter_floor_room){
			//every other do downstairs then down stairs
			mapxy(x,y) = stairCount % 2 == 0 ? terrain_type::ter_floor_downstairs : terrain_type::ter_floor_upstairs;
			stairCount++;
		}
	}
}

static void clearDungeon(dungeon_t *d, players_t *pl){
	delete[] d->rooms;
	delete[] pl->gameCharacters;
	pl->placedMonsterNames.clear();
	pl->placedObjectNames.clear();
}

static void makeDungeon(dungeon_t *d){
	makeBorder(d);
	int numRooms = rand() % 6 + 5;
	makeRooms(d, numRooms);
	makeStairs(d);
}

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

static int32_t game_event_cmp(const void *key, const void *with) {
  return ((game_event_t *) key)->turnTime - ((game_event_t *) with)->turnTime;
}

static void dijkstra_distance(dungeon_t *d, players_t *pl, int isNonTunnelling)
{
	static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
	static uint32_t initialized = 0;
	heap_t h;
	uint32_t x, y;

	if (!initialized) {
		for (y = 0; y < DUNGEON_Y; y++) {
			for (x = 0; x < DUNGEON_X; x++) {
				path[y][x].pos[dim_y] = y;
				path[y][x].pos[dim_x] = x;
			}
		}
		initialized = 1;
	}
  
	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			path[y][x].cost = INT_MAX;
		}
	}

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			ntmapxy(x,y) = 0;
		}
	}
	
	path[pl->pc.pos.y][pl->pc.pos.x].cost = 0;
	
	heap_init(&h, corridor_path_cmp, NULL);

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			if (mapxy(x, y) != terrain_type::ter_wall_immutable) {
					path[y][x].hn = heap_insert(&h, &path[y][x]);
				} else {
					path[y][x].hn = NULL;
				}
		}
	}

	while((p = (corridor_path_t*) heap_remove_min(&h))){
		if(isNonTunnelling == 0){
			tunmapxy(p->pos[dim_x], p->pos[dim_y]) = p->cost;
		}
		else{
			ntmapxy(p->pos[dim_x], p->pos[dim_y]) = p->cost;
		}

		//upper
		if ((path[p->pos[dim_y] - 1][p->pos[dim_x]].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         											p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y] - 1][p->pos[dim_x]].cost = p->cost + distancepair(p->pos);
			path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn);
		}
		//left
		if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         											p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]][p->pos[dim_x] - 1].hn);
		}
		//right
		if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&	(path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
													p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn);
		}
		//lower
		if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&	(path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
													p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost = p->cost + distancepair(p->pos);
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn);
		}
		//upper left
		if ((path[p->pos[dim_y] - 1][p->pos[dim_x]  - 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x]  - 1].cost >
													p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y] - 1][p->pos[dim_x]  - 1].cost =	p->cost + distancepair(p->pos);
			path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]  - 1].hn);
		}
		//lower left
		if ((path[p->pos[dim_y]  + 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y]  + 1][p->pos[dim_x] - 1].cost >
													p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y]   + 1][p->pos[dim_x] - 1].cost = p->cost + distancepair(p->pos);
			path[p->pos[dim_y]   + 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]   + 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]  + 1][p->pos[dim_x] - 1].hn);
		}
		//upper right
		if ((path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y]  - 1][p->pos[dim_x] + 1].cost >
													p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y]   - 1][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
			path[p->pos[dim_y]  - 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]  - 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]  - 1][p->pos[dim_x] + 1].hn);
		}
		//upper left
		if ((path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost >
													p->cost + distancepair(p->pos))) {
			path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost = p->cost + distancepair(p->pos);
			path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn);
		}
	}
	heap_delete(&h);
}


#define contains(vector, item) (std::find(vector.begin(), vector.end(), item) != vector.end())
static void makeMonster(dungeon_t *d, players_t *pl, int i){
	


	while(true){
		int monvar = rand() % Monsters.size();
		npc_template_t mon = Monsters[monvar];
		if(!mon.isUnique() || ((!contains(pl->placedMonsterNames, mon.name)) && (!contains(pl->deadMonsterNames, mon.name)))){
			int rarnum = rand() % 100 + 1;
			if(rarnum < mon.rarity){
				npc_t newmon = mon.generate();
				pair_xy_t arr[pl->num_chars];
				pl->fillPairArray(arr);
				newmon.pos = getRandomOpenLocation(d, arr, pl->num_chars, pl->pc.pos);
				pl->gameCharacters[i] = newmon;
				pl->placedMonsterNames.push_back(mon.name);
				// getch();	
				break;
				
			}
				
			
		}
		
	}
}

static void makeMonsters(dungeon_t *d, players_t *pl){
	pl->num_chars = numMonsters;
	for(int i = 0;i < numMonsters;i++){
		makeMonster(d, pl, i);
	}
}

static void makeObject(dungeon_t *d, players_t *pl, int i){

	while(true){
		int objvar = rand() % Objects.size();
		object_template_t obj = Objects[objvar];
		if(obj.artifactstatus != "TRUE" || ((!contains(pl->placedObjectNames, obj.name)) && (!contains(pl->destroyedObjectNames, obj.name)))){
			int rarnum = rand() % 100 + 1;
			if(rarnum < obj.rarity){
				game_object_t newobj = obj.generate();
				pair_xy_t arr[pl->num_chars];
				pl->fillPairArray(arr);
				newobj.pos = getRandomLocation(d);
				pl->gameObjects.push_back(newobj);
				pl->placedObjectNames.push_back(obj.name);
				// getch();	
				break;
				
			}
				
			
		}
		
	}
}

static void makeObjects(dungeon_t *d, players_t *pl){
	for (int i = 0; i < 10; i++){
		makeObject(d, pl, i);
	}

}


static void setupGame(dungeon_t *d, players_t *pl){
	
	makePlayerCharacter(d, pl);
	//game_event_t *ev = newEvent(d, &d->pc, 0);
	//heap_insert(&d->event_heap, &ev);
	//create monsters
	pl->gameCharacters = new npc_t[numMonsters];
	
	makeMonsters(d, pl);
	makeObjects(d,pl);
	dijkstra_distance(d, pl, 0); //tunnellers
	dijkstra_distance(d, pl, 1); //non tunnellers

	heap_init(&d->event_heap, game_event_cmp, NULL);

	
	//Add PC Event
	gevents[eventCounter].game_char = &pl->pc;
	gevents[eventCounter].turnTime = 0;
	heap_insert(&d->event_heap, &gevents[eventCounter]);
	eventCounter++;

	//Add Monster Events
	for(int i = 0;i < pl->num_chars;i++){
		gevents[eventCounter].game_char = &pl->gameCharacters[i];
		gevents[eventCounter].turnTime = 0;
		heap_insert(&d->event_heap, &gevents[eventCounter]);
		eventCounter++;
	}
}


static void movePC(dungeon_t *d, players_t *pl, int xOffset, int yOffset){
	pair_xy_t arr[pl->num_chars];
	pl->fillPairArray(arr);
	if(isCellOccupied(arr, pl->num_chars, pl->pc.pos.y, pl->pc.pos.x, pl->pc.pos, true)) {
		game_character_t *gc = getCharacterFromCell(pl, pl->pc.pos.y, pl->pc.pos.x);
		pl->pc.attack(gc);
		if(gc->dead){
			pl->deadMonsterNames.push_back(gc->name);
			if(gc->boss){
				d->gameStatus = game_status_t::game_status_won;
			}
		}
	}
	else{
		if(hardnessxy(pl->pc.pos.x + xOffset, pl->pc.pos.y + yOffset) == 0){
			pl->pc.pos.x = pl->pc.pos.x + xOffset;
			pl->pc.pos.y = pl->pc.pos.y + yOffset;
			dijkstra_distance(d, pl,0);
			dijkstra_distance(d, pl,1);
			if(doesCellHaveObject(pl, pl->pc.pos.x, pl->pc.pos.y)){
				if(pl->pc.carrySlots.size() < 10){
					game_object_t gObject = popObjectFromCell(pl, pl->pc.pos.x, pl->pc.pos.y);
					pl->pc.carrySlots.push_back(gObject);
				}

			}
		
		}
	} 
}

static void showMonsters(dungeon_t *d, players_t *pl){
	int scrollIndex = 0;
	int input = -1;
	printMonsters(pl, scrollIndex * MAX_MONSTER_LIST);
	while(input != 27){
		input = getch();
		switch(input){
			case KEY_UP:
				if(scrollIndex > 0){
					scrollIndex--;
				}
				printMonsters(pl, scrollIndex * MAX_MONSTER_LIST);
				break;
			case KEY_DOWN:
				if((scrollIndex + 1) * MAX_MONSTER_LIST < pl->num_chars ){
					scrollIndex++;
				}
				printMonsters(pl, scrollIndex * MAX_MONSTER_LIST);
				break;
		}
	}
	clear();
	printDungeon(d, pl);
}

static pair_xy_t runTeleport(dungeon_t *d, players_t *pl){
	pair_xy_t teleportPos = pl->pc.pos;
	while(true){
		printDungeon(d, pl);
		move(teleportPos.y + 1, teleportPos.x);
		addch('*');
		refresh();
		int input = getch();
		if(input == 't'){
			return teleportPos;
		}
		if( input == 'r'){
			teleportPos = getRandomCell(d);
			return teleportPos;
		}
		int xoffset;
		int yoffset;
		switch(input){
			case KEY_UP:
				xoffset = 0;
				yoffset = -1;
				break;
			case KEY_DOWN:
				xoffset = 0;
				yoffset = 1;
				break;
			case KEY_LEFT:
				xoffset = -1;
				yoffset = 0;
				break;
			case KEY_RIGHT:
				xoffset = 1;
				yoffset = 0;
				break;
		}
		
		if(teleportPos.y + yoffset >= 0 && teleportPos.y + yoffset < DUNGEON_Y &&
					teleportPos.x + xoffset >= 0 && teleportPos.x + xoffset < DUNGEON_X){
			if(mapxy(teleportPos.x + xoffset, teleportPos.y + yoffset) != terrain_type::ter_wall_immutable){
				teleportPos.y += yoffset;
				teleportPos.x += xoffset;
			}	
		}
	}
}

static void lookForMonster(dungeon_t *d, players_t *pl){
	pair_xy_t teleportPos = pl->pc.pos;
	int input = 0;
	while(input != 27){
		printDungeon(d, pl);
		move(teleportPos.y + 1, teleportPos.x);
		addch('*');
		refresh();
		input = getch();
		if(input == 't'){
			game_character_t *gamechar = getCharacterFromCell(pl, teleportPos.y, teleportPos.x);
			if(gamechar && !gamechar->isPC()){
				showMonster(gamechar);
				return;
			}
		}
		if(input == 27){
			return;
		}
		int xoffset;
		int yoffset;
		switch(input){
			case KEY_UP:
				xoffset = 0;
				yoffset = -1;
				break;
			case KEY_DOWN:
				xoffset = 0;
				yoffset = 1;
				break;
			case KEY_LEFT:
				xoffset = -1;
				yoffset = 0;
				break;
			case KEY_RIGHT:
				xoffset = 1;
				yoffset = 0;
				break;
		}
		
		if(teleportPos.y + yoffset >= 0 && teleportPos.y + yoffset < DUNGEON_Y &&
					teleportPos.x + xoffset >= 0 && teleportPos.x + xoffset < DUNGEON_X){
			if(mapxy(teleportPos.x + xoffset, teleportPos.y + yoffset) != terrain_type::ter_wall_immutable){
				teleportPos.y += yoffset;
				teleportPos.x += xoffset;
			}	
		}
	}
}

static void runPCEvent(dungeon_t *d, players_t *pl){
	int finished = 0;
	int newDungeon = 0;
	while(!finished){
		int input = getch();
		pair_xy_t tpos;
		size_t slot;
		string equipname;
		switch(input){
			case 'f':
				d->lightson = !d->lightson;
				printDungeon(d, pl);
				break;
			case '7':
			case 'y':
				movePC(d, pl, -1, -1);
				finished = 1;
				break;
			case '8':
			case 'k':
			case KEY_UP:
				movePC(d, pl, 0, -1);
				finished = 1;
				break;
			case '9':
			case 'u':
				movePC(d, pl, 1,1);
				finished = 1;
				break;
			case '4':
			case 'h':
			case KEY_LEFT:
				movePC(d, pl, -1,0);
				finished = 1;
				break;
			case '5':
			case ' ':
				finished = 1;
				break;
			case '6':
			case 'l':
			case KEY_RIGHT:
				movePC(d, pl, 1,0);
				finished = 1;
				break;
			case '1':
			case 'b':
				movePC(d, pl, -1, 1);
				finished = 1;
				break;
			case '2':
			case 'j':
			case KEY_DOWN:
				movePC(d, pl, 0,1);
				finished = 1;
				break;
			case '3':
			case 'n':
				movePC(d, pl, 1,1);
				finished = 1;
				break;
			case '>':
				if(mapxy(pl->pc.pos.x, pl->pc.pos.y) == terrain_type::ter_floor_downstairs){
					newDungeon = 1;
				}
				finished = 1;
				break;
			case '<':
				if(mapxy(pl->pc.pos.x, pl->pc.pos.y) == terrain_type::ter_floor_upstairs){
					newDungeon = 1;
				}
				finished = 1;
				break;
			case 'Q':
				quit = 1;
				finished = 1;
				break;
			case 'm':
				showMonsters(d, pl);
				break;
			case 't':
				tpos = runTeleport(d, pl);
				pl->pc.pos.x = tpos.x;
				pl->pc.pos.y = tpos.y;
				finished = 1;
				break;
			case 'w':
			//to wear
				slot = promptforCarrySlot(pl->pc);
				wearObject(pl->pc, slot);
				printDungeon(d, pl);
				break;
			case 'T':
			//t: to take off an item
				equipname = promptForEquipmentName(pl->pc);
				if(equipname != "")
					takeOffEquipment(pl, equipname);
				printDungeon(d, pl);
				break;
			case 'd':
			//d: to drop and item
				slot = promptforCarrySlot(pl->pc);
				dropObject(pl, slot);
				printDungeon(d, pl);
				break;
			case 'x':
			//expunge item from game
				slot = promptforCarrySlot(pl->pc);
				expungeObject(pl->pc, slot);
				printDungeon(d, pl);
				break;
			case 'i':
			//list PC inventory
				listPcInventory(pl->pc);
				printDungeon(d, pl);
				break;
			case 'e':
			//List PC equiptment
				listPcEquipment(pl->pc);
				printDungeon(d, pl);	
				break;
			case 'I':
			//inspect item
				slot = promptforCarrySlot(pl->pc);
				showObject(pl->pc, slot);
				printDungeon(d, pl);
				break;
			case 'L':
			//look at a monster
				lookForMonster(d, pl);
				printDungeon(d, pl);
				break;

		}

		if(quit)
			return;

		if(newDungeon){
			clearDungeon(d,pl);
			makeDungeon(d);
			setupGame(d, pl);
		}
		else{
			
		}
	}
}

//return 1 if success else 0
static uint8_t runGameEvent(dungeon_t *d, players_t *pl, game_event_t *gevent){
	//pc moves randomly into an adjacent position
	pair_xy_t newPos = pair_xy_t(0,0);
	//monster moves according to abilities
	if(gevent->game_char->isPC()){
		runPCEvent(d, pl);
		printDungeon(d, pl);
	}else{
		//monster
		//check for erratic first. 50% random chance of random movement
		newPos = gevent->game_char->getNextCell(d, pl->pc.pos);
		npc_t * npc = (npc_t*) (gevent->game_char);
		
		pair_xy_t arr[pl->num_chars];
		pl->fillPairArray(arr);
		if(isCellOccupied(arr, pl->num_chars, newPos.y, newPos.x, pl->pc.pos, false)){
			game_character_t *gc = getCharacterFromCell(pl, newPos.y, newPos.x);
			if(gc->isPC()){
				((pc_t*)gc)->attack(gevent->game_char);
				if(npc->dead){
					pl->deadMonsterNames.push_back(npc->name);
					if(npc->boss){
						d->gameStatus = game_status_t::game_status_won;
					}
				}
			}
			else {
				pair_xy_t temppos = npc->pos;
				npc->pos = gc->pos;
				gc->pos = temppos;
			}
		}
		
		if(npc->isIntelligent()){
			if(hasLineOfSightToPC(d, pl->pc.pos, npc->pos)){
				//save last know site to pc
				npc->lastSeenPC.x = pl->pc.pos.x;
				npc->lastSeenPC.y = pl->pc.pos.y;
				//if the character is not telepathic, need to copy the current distance map too
				if(npc->isTelepathic()){
					for(int y = 0;y < DUNGEON_Y;y++){
						for(int x = 0;x< DUNGEON_X;x++){
							npc->lastSeenDistancMap[y][x] = npc->canTunnel()
								? tunmapxy(x,y)
								: ntmapxy(x,y);
						}
					}
				}
			}
		}
		//printf("newPos is %d, %d\n", newPos.x, newPos.y);
		if(hardnessxy(newPos.x, newPos.y) > 0){
			if(npc->canTunnel()){
				hardnessxy(newPos.x, newPos.y) = hardnessxy(newPos.x, newPos.y) <= 85
					? 0
					: hardnessxy(newPos.x, newPos.y) - 85; 
				if(hardnessxy(newPos.x, newPos.y) == 0){
					//if we got the hardness down to zero, make it a cooridor
					//and move the character
					mapxy(newPos.x, newPos.y) = terrain_type::ter_floor_hall;
					npc->pos.x = newPos.x;
					npc->pos.y = newPos.y;
				}
				dijkstra_distance(d,pl,  0);
			}

		}else{
			npc->pos.x = newPos.x;
			npc->pos.y = newPos.y;
		}
			
		
		
	}

	pl->pc.updateSeenMap(d);

	return 1;
}

static void runGameEvents(dungeon_t *d, players_t *pl){
	
	game_event_t *gevent;
	uint8_t eventResult = -1;
	while(1 == 1){
		gevent = (game_event_t*)heap_remove_min(&d->event_heap);
		if(!gevent){
			//printf("gevent was null\n");
			break;
		}
		eventResult = runGameEvent(d, pl, gevent);
		if(quit){
			return;
		}
		if(eventResult == 0){
			//printf("eventREsult was 0");
			return;
		}
		if(d->gameStatus == game_status_t::game_status_won){
			return;
		}
		if(gevent->game_char->isPC()){
			printDungeon(d, pl);
			//printf("Monsters Left %d\n", monstersLeft(d));
			usleep(1000000);
		}else{
			if(pl->pc.dead){
				//printf("PC dead. Killed by %c\n", itohex(gevent->game_char->characterrain_type::ter_type));
				return;
			}
		}

		if(!gevent->game_char->dead){
			gevents[eventCounter].game_char = gevent->game_char;
			gevents[eventCounter].turnTime = gevent->turnTime + (1000/gevent->game_char->speed);
			heap_insert(&d->event_heap, &gevents[eventCounter]);
			eventCounter++;
		}
		
		if(eventCounter == 100000){
			//reset counter
			eventCounter = 0;
		}
	}
}


int main(int argc, char *argv[]){
	
	srand(time(NULL));

	string monsterfile = getGameDirectory();
	monsterfile += "/monster_desc.txt";
	Monsters = parseMonsterTemplates(monsterfile);
	if(Monsters.size() == 0){
		cout << "No monsters available for attacking" << endl;
	}
	
	string objectfile = getGameDirectory();
	objectfile += "/object_desc.txt";
	Objects = parseObjectTemplates(objectfile);
	

	int saveFlag = 0;
	int loadFlag = 0;
	dungeon_t d;
	players_t pl;

	std::string dir = getGameDirectory();
	//std::cout << "gamedir " << dir << std::endl;
	const char * gamedir = dir.c_str();
	mkdir(gamedir, S_IRWXU | S_IRWXG | S_IRWXO);

	char ch = ' ';
	while ((ch = getopt_long(argc, argv, "slyx", longopts, NULL)) != -1) {
        switch (ch) {
        case 's':
                saveFlag = 1;
                break;
        case 'l':
                loadFlag = 1;
                break;
		case 'y':
                yPC = atoi(optarg);
                break;
		case 'x':
                xPC = atoi(optarg);
                break;
		case 'n':
				numMonsters = atoi(optarg);
				break;
        }
	}

	//delete these after you confirmed they work
	//printf("Save Flag %d\n", saveFlag);
	//printf("Load Flag %d\n", loadFlag);

	
	//printf("yPos is %d\n", yPos);
	//printf("xPos is %d\n", xPos);
	//printf("numMon is %d\n", numMon);

	initscr();
	start_color();
	init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	
	if(loadFlag == 1){
		loadDungeon(&d);
	}
	else{
		makeDungeon(&d);
	}

	
	if(saveFlag == 1){
		saveDungeon(&d);
	}

	setupGame(&d, &pl);
	printDungeon(&d, &pl);
	runGameEvents(&d, &pl);
	printDungeon(&d, &pl);
	//printf("Monsters Left %d\n", monstersLeft(&d));
	
	
	endwin();
	
	printf("GAME STATUS: %s\n", d.gameStatus == game_status_t::game_status_won ? "WON!!" : (!pl.pc.dead ? "QUIT" : "LOST"));

	clearDungeon(&d, &pl);
	return 0;
}

void loadDungeon(dungeon_t *d){
	//check for dungeon file.  If it exists attempt to read it 
	//and create dungeon
	//otherwist just call makeDungeon to make one from scratch
	std::string gamefilename = getGameFilename();
	const char *filename = gamefilename.c_str();

	if( access( filename, F_OK ) == -1 ){
		//file doesnt exist
		//printf("Dungeon file doesnt exist.  Creating new Dungeon\n");
		makeDungeon(d);
		return;
	}

	FILE *gamefile = fopen(filename, "r");
	fseek(gamefile, 0, SEEK_END);
	long filelen = ftell(gamefile); 
	rewind(gamefile);
	//if file is at least 1700 bytes, we can continue
	//printf("filesize %d\n", filelen);
	if(filelen < 1700){
		//printf("Dungeon file should be at least 1700 bytes.  Creating new Dungeon\n");
		makeDungeon(d);
		return;
	}
	
	//read in bytes 16 - 19 to get the file size
	unsigned char sizeArray[4];
	fseek(gamefile, 16, SEEK_SET);
	fread(sizeArray,1,4, gamefile);


	unsigned int size;
	
	size = (sizeArray[0] << 24) | (sizeArray[1] << 16) | (sizeArray[2] << 8) | sizeArray[3];
	//printf("size array is %d %d %d %d\n",sizeArray[0], sizeArray[1], sizeArray[2], sizeArray[3]);
	//printf("int from code %d",size );

	int numOfRoomBytes = size - 1700;
	if(numOfRoomBytes % 4 != 0){
		//printf("Array is malformed.  room bytes not a multiple of 4.  Creating new dungeon\n");
		makeDungeon(d);
		return;
	}	

	makeBorder(d);
	
	//get hardness values
	unsigned char hardnessArray[1680];
	//fseek(gamefile, 20, SEEK_SET);
	fread(hardnessArray,1,1680,gamefile);
	
	for(int i = 0;i<1680;i++){
		/*if(i % 80 == 0){
			printf("loading hardness \n");
		}
		printf("%d (%d,%d) = %d ",i,i/80,i % 80, hardnessArray[i]);
		*/
		hardnessxy(i % 80, i / 80) = hardnessArray[i]; 
	}

	//printf("numOfRoomBytes is %d\n", numOfRoomBytes);
	unsigned char roomsArray[numOfRoomBytes];
	//fseek(gamefile, 1700, SEEK_SET);
	
	fread(roomsArray,1,numOfRoomBytes,gamefile);
	Room roomsFromFile[numOfRoomBytes / 4];
	
	for(int i = 0;i < numOfRoomBytes / 4;i++){
		roomsFromFile[i] = (Room){roomsArray[1 + (4 * i)], roomsArray[0 + (4 * i)], roomsArray[3 + (4 * i)], roomsArray[2 + (4 * i)]};
	}
	//printf("about assign num rooms");
	d->num_rooms = numOfRoomBytes / 4;
	d->rooms = new Room[d->num_rooms];
	for(int i = 0;i < d->num_rooms;i++){
		d->rooms[i].x = roomsFromFile[i].x;
		d->rooms[i].y = roomsFromFile[i].y;
		d->rooms[i].w = roomsFromFile[i].w;
		d->rooms[i].h = roomsFromFile[i].h;
	}
	
	fillRooms(d);
	
	//cooridors are anything with a hardness of 0 that is not already a room
	for(int y = 0;y< DUNGEON_Y;y++){
		for(int x=0;x < DUNGEON_X;x++){
			//printf("%d ",hardness[i][j]);
			//printf("=%c", dungeon[i][j]);
			if(hardnessxy(x,y) == 0){
				switch(mapxy(x,y)){
					case terrain_type::ter_floor_room:
						break;
					default:
						mapxy(x,y) = terrain_type::ter_floor_hall;
				}
			}
			//printf("\n");
		}
	}
}

//return 0 if dungeon is 50% rock or less, else return 1
int checkRooms(dungeon_t *d){
	int rock = 0;
	for(int y = 1; y < DUNGEON_Y - 1; y++){
		for(int x = 1; x < DUNGEON_X - 1;x ++){
			if(mapxy(x,y) == terrain_type::ter_wall){
				rock++;
			}
		}
	}
	//printf("Checking Room.  Rock at %d of %d\n", rock, 78 * 19);
	float fullness = rock / (78.0f * 19.0f);//TODO convert to float from constants
	//printf("fullness %f\n", fullness);
	if(fullness <= .50){
		return 0;
	}else{
		return 1;
	}
}

void saveDungeon(dungeon_t *d){
	std::string gamefilename = getGameFilename();
	const char *filename = gamefilename.c_str();


	//calculate size in bytes, which is simply 1700 + 4 * num of rooms;
	u_int32_t size = 1700 + 4 * d->num_rooms;
	//declare array to hold our data
	unsigned char game[size];
	game[0] = 'R';
	game[1] = 'L';
	game[2] = 'G';
	game[3] = '3';
	game[4] = '2';
	game[5] = '7';
	game[6] = '-';
	game[7] = 'S';
	game[8] = '2';
	game[9] = '0';
	game[10] = '1';
	game[11] = '8';
	//version
	game[12] = 0;
	game[13] = 0;
	game[14] = 0;
	game[15] = 0;
	//file size (big endian), turn int into a byte array
	u_int32_t sizeArray[4] = { (size >> 24) & 255, (size >> 16) & 255, (size >> 8) & 255, size & 255 };
	game[16] = sizeArray[0];
	game[17] = sizeArray[1];
	game[18] = sizeArray[2];
	game[19] = sizeArray[3];
	//printf("size array is %d %d %d %d\n",sizeArray[0], sizeArray[1], sizeArray[2], sizeArray[3]);
	//printf("game array is %d %d %d %d\n",game[16], game[17], game[18], game[19]);
	//hardness
	
	for(int y = 0;y < DUNGEON_Y;y++){
		//printf("saving hardness");
		for(int x = 0;x < DUNGEON_X;x++){
			game[(DUNGEON_X * y) + x + 20] = hardnessxy(x,y);
			
			//printf("%d =%d ",(80 * i) + j + 20, hardness[i][j] & 0xFF);
		}
		//printf("\n");
	}
	//room data
	for(int i = 0;i < d->num_rooms;i++){
		game[1700 + (i * 4)] = d->rooms[i].y;
		game[1701 + (i * 4)] = d->rooms[i].x;
		game[1702 + (i * 4)] = d->rooms[i].h;
		game[1703 + (i * 4)] = d->rooms[i].w;
	}
	
	//write to file
	FILE *file = fopen(filename, "wb");
	fwrite(game, 1, size, file);
	fclose(file);
}

std::string getGameDirectory(){
	std::string result = "";
	result += getenv("HOME");
	result += "/.rlg327";
	return result;
}

std::string getGameFilename(){
	std::string result2 = "";
	result2 += getGameDirectory();
	result2 += "/dungeon";
	return result2;
}
