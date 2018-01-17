#include <stdio.h>


char dungeon[21][80];
void makeBorder();
void printDungeon();





int main(){
	makeBorder();
	printDungeon();
	return 0;
}


void makeBorder(){
	for(int i = 0; i < 80; i++){
		dungeon[0][i] = '-';
		dungeon[21][i] = '-';

	}
	for(int i = 1; i < 20; i++){
		dungeon[i][0] = '|';
		dungeon[i][79] = '|';
	}
}

void printDungeon(){
	for(int i = 0; i <= 21; i++){
	      for(int j = 0; j < 80; i++){
	      	printf("%d", dungeon[i][j]);
		}

		printf("\n");

	}


}
