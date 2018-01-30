#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "room.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h> 
#include <machine/endian.h>

#define ROOMFLOOR '.'
#define COORIDORFLOOR '#'


char dungeon[21][80];
void makeBorder();
void printDungeon();
void makeCoorridor(Room a, Room b);
Room makeRoom(int w, int h);
void makeRooms(int numRooms);
int checkRooms();
void fillRoom(Room r);
unsigned char hardness[21][80];
void saveDungeon(Room rooms[], int numOfRooms);
void loadDungeon();
int saveFlag = 0;
int loadFlag = 0;
char *getGameDirectory();
char *getGameFileName();
void writer(int numRooms);
 
/* options descriptor  for --save and --load*/
static struct option longopts[] = {
	{ "save", no_argument, NULL, 's' },
        { "load", no_argument, NULL,'l'}
};




int main(int argc, char *argv[]){
    
	char ch = ' ';
  	while ((ch = getopt_long(argc, argv, "sl", longopts, NULL)) != -1) {
       	  switch (ch) {
       	  case 's':
                saveFlag = 1;
                break;
       	 case 'l':
                loadFlag = 1;
                break;
       	 }
   	}
 
   	//delete these after you confirmed they work
  	printf("Save Flag %d\n", saveFlag);
   	printf("Load Flag %d\n", loadFlag);
	srand(time(NULL));
	makeBorder();
	int numRooms = rand() % 6 + 5;
	makeRooms(numRooms);
	printDungeon();

	
	return 0;
}


void makeBorder(){
	for(int i = 0; i < 80; i++){
		dungeon[0][i] = '-';
		dungeon[20][i] = '-';
		hardness[0][i] = 255;
		hardness[20][i] = 255;



	}
	for(int i = 1; i < 20; i++){
		dungeon[i][0] = '|';
		dungeon[i][79] = '|';
		hardness[i][0] = 255;
		hardness[0][79] = 255;
		for(int j = 1; j < 79; j++){
			hardness[i][j] = rand() % 254 + 1;
			dungeon[i][j] = ' ';
		}
	}
}

void printDungeon(){
	for(int i = 0; i < 21; i++){
		for(int j = 0; j < 80; j++){
	      	printf("%c", dungeon[i][j]);
		}

		printf("\n");

	}


}

void makeRooms(int numRooms){
    //printf("Attempting to make %d rooms\n", numRooms);
    Room rooms[numRooms];
    //intialize all rooms with zeros
    /*for(int i = 0;i < numRooms; i++){
        Room rm = {0,0,0,0};
        rooms[i] = rm;
    }*/
    for(int i = 0;i < numRooms; i++){
        //printf("Room number %d\n",i);
        if(checkRooms() == 0){
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
            createdRoom = makeRoom(roomW, roomH);
            attempts++;
        }
 
        if(createdRoom.w == 0){
            //we gotta zero width room, which means the room couldnt be created.
            //room not success, just add the zeroed room to the list and we'll remove it later.
            rooms[i] = createdRoom;
        }
	else{
            //printf("Room Success.%d x %d at %d, %d\n", createdRoom.w, createdRoom.h, createdRoom.x, createdRoom.y);
            rooms[i] = createdRoom;
            fillRoom(createdRoom);
        }
 
    }
 
    //remove zeroed rooms
    //count non zeroed rooms
    
    int filledRoomsNum = 0;
    for (int i = 0; i < numRooms; i++){
        if(rooms[i].w > 0){
            filledRoomsNum++;
        }
    }
    //create new rooms array with filled rooms only
    Room filledRooms[filledRoomsNum];
    for (int i = 0; i < numRooms; i++){
        if(rooms[i].w > 0){
            filledRooms[i] = rooms[i];
        }
    }
 
    //make coorridors
    for (int i = 0; i < filledRoomsNum-1; i++){
        
        makeCoorridor(filledRooms[i], filledRooms[i + 1]);
    }
 
    //save if specified
    //TODO  uncomment once saveFlag and saveDungeon are implemented.
/*
    if(saveFlag == 1){
        saveDungeon(filledRooms, filledRoomsNum);
    }
    */
}

//attempt to make a room.  return a room for success, else a room with all 0s;
Room makeRoom(int w, int h){
		//pick a random spot in the grid and see if the room fits
	//and does not overlap any rooms or are within 1 cell of another room
	int posx = rand() % 76 + 2; //2 to 77
	int posy = rand() % 17 + 2; //2 to 18
	//printf("Attempting room of %d x %d at %d,%d\n", w, h, posx, posy);
	if(posx + w <= 77 && posy + h <= 18){
		//room fits.  now check for overlap plus one on each side
		//printf("room Fits\n");
		for(int i = posy - 1;i < posy + h + 1;i++){
			for(int j = posx - 1;j < posx + w + 1;j++){
				//printf("cell at %d, %d, is \"%c\"\n",j, i, dungeon[i][j]);
				if(dungeon[i][j] == ' '){
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

void fillRoom(Room r){
	//fill room with .'s
	//printf("Filling room of %d x %d\n", r.w, r.h);
	for(int i = r.y;i < r.y + r.h;i++){
		for(int j = r.x;j < r.x + r.w;j++){
			//printf("Filling %d, %d with a period\n",j,i);
			dungeon[i][j] = '.';
			hardness[i][j] = 0;
		}

	}
}

//return 0 if dungeon is 50% rock or less, else return 1
int checkRooms(){
	int rock = 0;
	for(int i = 1; i < 20; i++){
		for(int j = 1; j < 79;j ++){
			if(dungeon[i][j] == ' '){
				rock++;
			}
		}
	}
	//printf("Checking Room.  Rock at %d of %d\n", rock, 78 * 19);
	float fullness = rock / (78.0f * 19.0f);
	//printf("fullness %f\n", fullness);
	if(fullness <= .10){
		return 0;
	}
	else{
		return 1;
	}
}

void makeCoorridor(Room a, Room b){
	
	if (a.x > b.x){

		for(int i = a.x; i>b.x; i--){
			if (dungeon[a.y][i] != ROOMFLOOR){
				dungeon[a.y][i] = COORIDORFLOOR;
				hardness[a.y][i] = 0;
			}
		}
	}

	else{

		for(int i = a.x; i<b.x; i++){
			if (dungeon[a.y][i] != ROOMFLOOR){
				dungeon[a.y][i] = COORIDORFLOOR;
				hardness[a.y][i] = 0;

			}
		}

	}

	if (a.y > b.y){

		for(int i = a.y; i>b.y; i--){
			if (dungeon[i][b.x] != ROOMFLOOR){
     			dungeon[i][b.x] = COORIDORFLOOR;
			hardness[i][b.x] = 0;
			}

		}
	}


	else{
		for(int i = a.y; i<b.y; i++){
     			 if (dungeon[i][b.x] != ROOMFLOOR){
	      			dungeon[i][b.x] = COORIDORFLOOR;
			      	hardness[i][b.x] = 0;
	
		      	 }
		}

	}


}
void saveDungeon(Room rooms[], int numOfRooms){

}
void loadDungeon(){


}
char* getGameDirectory(){
	char *gamedir = "./rlg327";
	char *home = getenv("HOME");
	char *finaldir = malloc(strlen(home)+strlen(gamedir)+1);
	strcpy(finaldir, home);
	strcat(finaldir, gamedir);

	return finaldir;
}

char* getGameFilename(){
	char *finaldir = getGameDirectory();
	char *fname = "/dungeon";
	char *filename = malloc(strlen(finaldir)+strlen(fname)+1);
	strcpy(filename, finaldir);
	strcat(filename, fname);

	return filename;
}

/*void writer(int numRooms){
	char semantic[] = "RLG327-S2018";
	fwrite(semantic, sizeof(char), 12, f);

	int version = 0;
	int be;
	be = htobe32(version);
	fwrite(& be, sizeof(int), 1, f);

	int size = 12 + 4+4+ 80*21 + 4*numRooms;
	be = htobe32(size);
	fwrite(& be, sizeof(int), 1,f);
	fwrite(hardness, 1, 80*21,f);

	size = numRooms*4;//size of rooms array
	be = htobe32(size);
	fwrite(& be, sizeof(int), 1, f);

}*/



