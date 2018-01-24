#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "room.h"

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




int main(){
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

	}
	for(int i = 1; i < 20; i++){
		dungeon[i][0] = '|';
		dungeon[i][79] = '|';
		for(int j = 1; j < 79; j++){
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
	printf("Attempting to make %d rooms\n", numRooms);
	Room rooms[numRooms];
	//intialize all rooms with zeros
	/*for(int i = 0;i < numRooms; i++){
		Room rm = {0,0,0,0};
		rooms[i] = rm;
	}*/
	for(int i = 0;i < numRooms; i++){
		printf("Room number %d\n",i);
		if(checkRooms() == 0){
			printf("dungeon full\n");
			return;
		}
		
		//make a new room of at least 3 x 2 up to 10 x 10
		int roomH = rand() % 9 + 2;
		int roomW = rand() % 8 + 3;
		printf("new dimensions %d x %d\n", roomH, roomW);
		int attempts = 1;
		///try 2000 times to make a room
		Room createdRoom;
		printf("createdRoom 2nd pass %d\n", createdRoom.w);
		while(attempts < 2000 && (createdRoom.w == 0 || attempts == 1)){
			createdRoom = makeRoom(roomW, roomH);
			attempts++;
		}

		if(createdRoom.w == 0){
			//room not success....stop
			printf("Room Failure\n");
			return;
		}else{
			printf("Room Success.%d x %d at %d, %d\n", createdRoom.w, createdRoom.h, createdRoom.x, createdRoom.y);
			rooms[i] = createdRoom;
			fillRoom(createdRoom);
		}
	}
	for (int i = 0; i < numRooms-1; i++){
		makeCoorridor(rooms[i], rooms[i + 1]);
	}
	
}

//attempt to make a room.  return a room for success, else a room with all 0s;
Room makeRoom(int w, int h){
		//pick a random spot in the grid and see if the room fits
	//and does not overlap any rooms or are within 1 cell of another room
	int posx = rand() % 76 + 2; //2 to 77
	int posy = rand() % 17 + 2; //2 to 18
	printf("Attempting room of %d x %d at %d,%d\n", w, h, posx, posy);
	if(posx + w <= 77 && posy + h <= 18){
		//room fits.  now check for overlap plus one on each side
		printf("room Fits\n");
		for(int i = posy - 1;i < posy + h + 1;i++){
			for(int j = posx - 1;j < posx + w + 1;j++){
				printf("cell at %d, %d, is \"%c\"\n",j, i, dungeon[i][j]);
				if(dungeon[i][j] == ' '){
					//all good
				}
				else{
					return (Room){0,0,0,0};
				}
			}

		}
		printf("Room All Clear\n");
		printf("Room Made\n");
		return (Room){posx, posy,w, h};
	}

	return (Room) {0,0,0,0};
	
}

void fillRoom(Room r){
	//fill room with .'s
	printf("Filling room of %d x %d\n", r.w, r.h);
	for(int i = r.y;i < r.y + r.h;i++){
		for(int j = r.x;j < r.x + r.w;j++){
			printf("Filling %d, %d with a period\n",j,i);
			dungeon[i][j] = '.';
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
	printf("Checking Room.  Rock at %d of %d\n", rock, 78 * 19);
	float fullness = rock / (78.0f * 19.0f);
	printf("fullness %f\n", fullness);
	if(fullness <= .10){
		return 0;
	}else{
		return 1;
	}
}

void makeCoorridor(Room a, Room b){
	
	if (a.x > b.x){

		for(int i = a.x; i>b.x; i--){
			if (dungeon[a.y][i] != ROOMFLOOR){
				dungeon[a.y][i] = COORIDORFLOOR;
			}
		}
	}

	else{

		for(int i = a.x; i<b.x; i++){
			if (dungeon[a.y][i] != ROOMFLOOR){
				dungeon[a.y][i] = COORIDORFLOOR;
			}
		}

	}

	if (a.y > b.y){

		for(int i = a.y; i>b.y; i--){
			if (dungeon[i][b.x] != ROOMFLOOR){
     			dungeon[i][b.x] = COORIDORFLOOR;
		}
	}
}

	else{
		for(int i = a.y; i<b.y; i++){
     			 if (dungeon[i][b.x] != ROOMFLOOR){
	      			dungeon[i][b.x] = COORIDORFLOOR;    }
		}

	}


}



