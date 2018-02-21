#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "room.h"
#include "heap.h"


#define DUNGEON_X              80
#define DUNGEON_Y              21

#define ROOMFLOOR '.'
#define COORIDORFLOOR '#'
#define PCFLOOR '@'

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
		{ "x", required_argument, NULL,'x'}
};

//unsigned char dungeon[21][80];
//unsigned char hardness[21][80];
//unsigned char distMap[21][80];
//unsigned char distSimpleMap[21][80];

#define mapxy(x, y) (d->map[y][x])
#define ntmapxy(x, y) (d->ntmap[y][x])
#define tunmapxy(x, y) (d->tunmap[y][x])
#define ntpair(pair) (d->ntmap[pair[dim_y]][pair[dim_x]])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
#define distancepair(pair) (d->hardness[pair[dim_y]][pair[dim_x]]/85 +1)
#define hardnessxy(x, y) (d->hardness[y][x])

void makeBorder(dungeon_t *d);
void printDungeon(dungeon_t *d);
void makeCoorridors(dungeon_t *d);
void makeCoorridor(dungeon_t *d, Room a, Room b);
Room makeRoom(dungeon_t *d, int w, int h);
void makeRooms(dungeon_t *d, int numRooms);
int checkRooms(dungeon_t *d);
void fillRooms(dungeon_t *d);
void fillRoom(dungeon_t *d, Room r);
void makeDungeon(dungeon_t *d);
void saveDungeon(dungeon_t *d);
void loadDungeon(dungeon_t *d);
char* getGameDirectory();
char* getGameFilename();
void printHardness();
void makeRandomPlayerCharacter(dungeon_t *d);
void makePlayerCharacter(dungeon_t *d, pair_t pos);

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

static void dijkstra_distance(dungeon_t *d, int isNonTunnelling)
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
	
	path[d->pc[dim_y]][d->pc[dim_x]].cost = 0;
	
	heap_init(&h, corridor_path_cmp, NULL);

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			if(isNonTunnelling == 0){
				//tunnellers
				if (mapxy(x, y) != ter_wall_immutable) {
					path[y][x].hn = heap_insert(&h, &path[y][x]);
				} else {
					path[y][x].hn = NULL;
				}
			}else{
				//non tunnellers
				if (hardnessxy(x,y) == 0){
					path[y][x].hn = heap_insert(&h, &path[y][x]);
				} else {
					path[y][x].hn = NULL;
				}
			}
		}
	}

	while((p = heap_remove_min(&h))){
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

int main(int argc, char *argv[]){

	int saveFlag = 0;
	int loadFlag = 0;
	int yPos = -1;
	int xPos = -1;
	dungeon_t d;	
	char *gamedir;

	gamedir = getGameDirectory();
	mkdir(gamedir, S_IRWXU | S_IRWXG | S_IRWXO);
	free(gamedir);

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
                yPos = atoi(optarg);
                break;
		case 'x':
                xPos = atoi(optarg);
                break;
        }
	}

	//delete these after you confirmed they work
	//printf("Save Flag %d\n", saveFlag);
	//printf("Load Flag %d\n", loadFlag);

	
	//printf("yPos is %d\n", yPos);
	//printf("xPos is %d\n", xPos);

	srand(time(NULL));


	
	if(loadFlag == 1){
		loadDungeon(&d);
	}
	else{
		makeDungeon(&d);
	}

	
	if(saveFlag == 1){
		saveDungeon(&d);
	}

	pair_t pos;
	pos[dim_y] = yPos;
	pos[dim_x] = xPos;
	makePlayerCharacter(&d, pos);
	dijkstra_distance(&d, 0); //tunnellers
	dijkstra_distance(&d, 1); //non tunnellers


	printDungeon(&d);
	free(d.rooms);
	return 0;
}

void makeDungeon(dungeon_t *d){
	makeBorder(d);
	int numRooms = rand() % 6 + 5;
	makeRooms(d, numRooms);
}

void loadDungeon(dungeon_t *d){
	//check for dungeon file.  If it exists attempt to read it 
	//and create dungeon
	//otherwist just call makeDungeon to make one from scratch
	char *filename = getGameFilename();

	if( access( filename, F_OK ) == -1 ){
		//file doesnt exist
		printf("Dungeon file doesnt exist.  Creating new Dungeon\n");
		makeDungeon(d);
		return;
	}

	FILE *gamefile = fopen(filename, "r");
	free(filename);
	fseek(gamefile, 0, SEEK_END);
	long filelen = ftell(gamefile); 
	rewind(gamefile);
	//if file is at least 1700 bytes, we can continue
	//printf("filesize %d\n", filelen);
	if(filelen < 1700){
		printf("Dungeon file should be at least 1700 bytes.  Creating new Dungeon\n");
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
		printf("Array is malformed.  room bytes not a multiple of 4.  Creating new dungeon\n");
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
	d->rooms = malloc(sizeof(roomsFromFile));
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
					case ter_floor_room:
						break;
					default:
						mapxy(x,y) = ter_floor_hall;
				}
			}
			//printf("\n");
		}
	}
}


void makeBorder(dungeon_t *d){
	for(int y = 0;y < DUNGEON_Y;y++){
		for(int x = 0;x < DUNGEON_X;x++){
			if(y == 0 || y == DUNGEON_Y -1 || x == 0 || x == DUNGEON_X - 1){
				mapxy(x, y) = ter_wall_immutable;
				hardnessxy(x,y) = 255;
			}else{
				mapxy(x,y) = ter_wall;
				hardnessxy(x,y) =  rand() % 254 + 1; //random hardness from 1 to 254
			}
		}
	}
}

void printDungeon(dungeon_t *d){
	printf("Dungeon\n");
	for (int y = 0; y < DUNGEON_Y; y++) {
		for (int x = 0; x < DUNGEON_X; x++) {
			switch (mapxy(x,y)) {
				case ter_wall:
					putchar(' ');
					break;
				case ter_wall_immutable:
					if(y == 0 || y == DUNGEON_Y - 1)
						putchar('-');
					else if(x == 0 || x == DUNGEON_X - 1)
						putchar('|');
					else
						putchar(' ');
					break;
				case ter_floor:
				case ter_floor_room:
					putchar(ROOMFLOOR);
					break;
				case ter_floor_hall:
					putchar(COORIDORFLOOR);
					break;
				case ter_player:
					putchar(PCFLOOR);
					break;
					/*
				case ter_debug:
					putchar('*');
					fprintf(stderr, "Debug character at %d, %d\n", x, y);
					break;
					*/
			}
    	}
    	putchar('\n');
	}

  	printf("Non Tunnelers Map\n");
	for (int y = 0; y < DUNGEON_Y; y++) {
		for (int x = 0; x < DUNGEON_X; x++) {
			switch (mapxy(x,y)) {
				case ter_player:
					putchar(PCFLOOR);
					break;
				case ter_wall_immutable:
					if(y == 0 || y == DUNGEON_Y - 1)
						putchar('-');
					else if(x == 0 || x == DUNGEON_X - 1)
						putchar('|');
					else
						putchar(' ');
					break;
				default:
					if(ntmapxy(x,y) != 0){
						printf("%d", ntmapxy(x,y) % 10);
					}else
						putchar(' ');
			}
    	}
    	putchar('\n');
	}

	printf("Tunnelers Map\n");
	for (int y = 0; y < DUNGEON_Y; y++) {
		for (int x = 0; x < DUNGEON_X; x++) {
			switch (mapxy(x,y)) {
				case ter_player:
					putchar(PCFLOOR);
					break;
				case ter_wall_immutable:
					if(y == 0 || y == DUNGEON_Y - 1)
						putchar('-');
					else if(x == 0 || x == DUNGEON_X - 1)
						putchar('|');
					else
						putchar(' ');
					break;
				default:
					if(tunmapxy(x,y) != 0){
						printf("%d", tunmapxy(x,y) % 10);
					}else
						putchar(' ');
			}
    	}
    	putchar('\n');
	}
}


void makeRooms(dungeon_t *d, int numRooms){
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
	d->rooms = malloc(sizeof (actualRooms));
	for(int i = 0;i < d->num_rooms;i++){
		d->rooms[i].x = actualRooms[i].x;
		d->rooms[i].y = actualRooms[i].y;
		d->rooms[i].w = actualRooms[i].w;
		d->rooms[i].h = actualRooms[i].h;
	}
	fillRooms(d);
	makeCoorridors(d);
	
}

//attempt to make a room.  return a room for success, else a room with all 0s;
Room makeRoom(dungeon_t *d, int w, int h){
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
				if(mapxy(x, y) == ter_wall){
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

void fillRooms(dungeon_t *d){
	for(int i = 0;i < d->num_rooms;i++){
		fillRoom(d, d->rooms[i]);
	}
}

void fillRoom(dungeon_t *d, Room r){
	//fill room with .'s
	//printf("Filling room of %d x %d\n", r.w, r.h);
	for(int y = r.y;y < r.y + r.h;y++){
		for(int x = r.x;x < r.x + r.w;x++){
			//printf("Filling %d, %d with ter_floor_room\n",x, y);
			mapxy(x,y) = ter_floor_room;
			hardnessxy(x,y) = 0;
		}

	}
}

//return 0 if dungeon is 50% rock or less, else return 1
int checkRooms(dungeon_t *d){
	int rock = 0;
	for(int y = 1; y < DUNGEON_Y - 1; y++){
		for(int x = 1; x < DUNGEON_X - 1;x ++){
			if(mapxy(x,y) == ter_wall){
				rock++;
			}
		}
	}
	//printf("Checking Room.  Rock at %d of %d\n", rock, 78 * 19);
	float fullness = rock / (78.0f * 19.0f);//TODO convert to float from constants
	//printf("fullness %f\n", fullness);
	if(fullness <= .10){
		return 0;
	}else{
		return 1;
	}
}

void makeCoorridors(dungeon_t *d){
	for(int i = 0;i < d->num_rooms - 1; i++){
		makeCoorridor(d, d->rooms[i], d->rooms[i + 1]);
	}
}

void makeCoorridor(dungeon_t *d, Room a, Room b){
	//printf("Making Cooridor from room {(%d,%d) of %d x %d} to room {(%d,%d) of %d x %d}\n", a.x, a.y, a.w, a.h, b.x, b.y, b.w, b.h);
	
	if (a.x > b.x){

		for(int x = a.x; x>b.x; x--){
			if (mapxy(x, a.y) != ter_floor_room){
				mapxy(x, a.y) = ter_floor_hall;
				hardnessxy(x, a.y) = 0;
			}
		}
	}

	else{

		for(int x = a.x; x<b.x; x++){
			if (mapxy(x, a.y) != ter_floor_room){
				mapxy(x, a.y) = ter_floor_hall;
				hardnessxy(x, a.y) = 0;
			}
		}

	}

	if (a.y > b.y){

		for(int y = a.y; y>b.y; y--){
			if (mapxy(b.x, y) != ter_floor_room){
     			mapxy(b.x, y) = ter_floor_hall;
				hardnessxy(b.x, y) = 0;
			}	
		}
	}

	else{
		for(int y = a.y; y<b.y; y++){
     			if (mapxy(b.x, y) != ter_floor_room){
	      			mapxy(b.x, y) = ter_floor_hall;
					hardnessxy(b.x, y)= 0;
				}
		}

	}


}

void saveDungeon(dungeon_t *d){
	char *filename = getGameFilename();
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
	free(filename);
}

char* getGameDirectory(){
	char *gamedir = "/.rlg327";
	char *home = getenv("HOME");
	char *finaldir = malloc(strlen(home) + strlen(gamedir) + 1);
	strcpy(finaldir, home);
	strcat(finaldir, gamedir);

	return finaldir;
}

char* getGameFilename(){
	char *finaldir = getGameDirectory();
	char *fname = "/dungeon";
	char *filename = malloc(strlen(finaldir) + strlen(fname) + 1);
	strcpy(filename, finaldir);
	strcat(filename, fname);

	return filename;
}


void makePlayerCharacter(dungeon_t *d, pair_t pos){
	if(pos[dim_y] > 0 && pos[dim_y] < DUNGEON_Y - 1 && pos[dim_x] > 0 && pos[dim_x] < DUNGEON_X - 1){
		if(mapxy(pos[dim_x], pos[dim_y]) == ter_floor_room){
			//go ahead place the character
			mapxy(pos[dim_x], pos[dim_y]) = ter_player;
			d->pc[dim_x] = pos[dim_x];
			d->pc[dim_y] = pos[dim_y];
		}else{
			makeRandomPlayerCharacter(d);
		}
	}else{
		makeRandomPlayerCharacter(d);
	}
}

void makeRandomPlayerCharacter(dungeon_t *d){
	//find a random floor spot for the PC
	int done = 0;
	while(done == 0){
		int yRand = rand() % 19 + 1;
		int xRand = rand() % 78 + 1;
		if(mapxy(xRand, yRand) == ter_floor_room){
			mapxy(xRand, yRand)= ter_player;
			d->pc[dim_x] = xRand;
			d->pc[dim_y] = yRand;
			done = 1;
		}
	}
}


