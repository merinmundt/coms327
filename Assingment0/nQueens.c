#include <stdio.h>

int grid[8][8];
void setZero();
void markQueen(int row, int col);
void unmarkQueen(int row, int col);

int numQueens;

int main(){
	
	
		for (int first = 0; first < 8; first++){
			if(grid[1][first] == 0){
				markQueen(1,first);
				
			//	
			}
			for(int second = 0; second < 8; second++){
				if(grid[2][second] == 0){
					markQueen(2,second);
				}

				for(int third = 0; third < 8; third++){
					if(grid[3][third] == 0){
						markQueen(3,third);
					}

					for(int fourth = 0; fourth < 8; fourth++){
						if(grid[4][fourth] == 0){
							markQueen(4,fourth);
						}

						for(int fifth = 0; fifth < 8; fifth++){
							if(grid[5][fifth] == 0){
								markQueen(5,fifth);
							}

							for(int sixth = 0; sixth < 8; sixth++){
								if(grid[6][sixth] == 0){
									markQueen(6,sixth);
								}

								for(int seventh = 0; seventh < 8; seventh++){
									if(grid[7][seventh] == 0){
										markQueen(7,seventh);
									}

									for(int eight = 0; eight < 8; eight++){
										if(grid[8][eight] == 0){
											markQueen(8,eight);
										}
										printf("a%db%dc%dd%de%df%dg%dh%d", first, second, third, fourth, fifth, sixth, seventh, eight);
										
									}
									unmarkQueen(1,seventh);

								}
								unmarkQueen(6,sixth);

							}
							unmarkQueen(5,fifth);
							
						}
						unmarkQueen(4,fourth);

					}
					unmarkQueen(3,third);

				}
				unmarkQueen(2,second);

			}
			unmarkQueen(1,first);
			
		}
return 0;		
}		
//unmarking the queen to make sure that the solution will reset when unfinished	
void unmarkQueen(int row, int col){
	for(int i = row; i < 8; i++){
		grid[i][col] = 0;
	}
	for(int i = row; i > 0; i--){
		grid[i][col] = 0;
	}
	for(int j = col; j < 8; j++){
		grid[row][j] = 0;
	}
	for(int j = col; j > 0; j--){
		grid[row][j] = 0;
	}
	for(int i = row; i < 8; i++){
       		grid[i][col] = 0;
		col++;
	}
	for(int i = row; i > 0; i--){
       		grid[i][col] = 0;
		col--;
	}
	for(int i = col; i < 8; i++){
       		grid[row][i] = 0;
		row++;
	}
	for(int i = col; i > 0; i--){
       		grid[row][i] = 0;
		row--;
	}
}
	
//marking the rows and columns and diagonals of the placed queen	
void markQueen(int row, int col){
	for(int i = row; i < 8; i++){
		grid[i][col] = 1;
	}
	for(int i = row; i > 0; i--){
		grid[i][col] = 1;
	}
	for(int j = col; j < 8; j++){
		grid[row][j] = 1;
	}
	for(int j = col; j > 0; j--){
		grid[row][j] = 1;
	}
	for(int i = row; i < 8; i++){
       		grid[i][col] = 1;
		col++;
	}
	for(int i = row; i > 0; i--){
       		grid[i][col] = 1;
		col--;
	}
	for(int i = col; i < 8; i++){
       		grid[row][i] = 1;
		row++;
	}
	for(int i = col; i > 0; i--){
       		grid[row][i] = 1;
		row--;
	}
	


	





}


//Sets everything to 0
void setZero(){

	for(int i= 0; i < 8; i++){
		for(int j=0; j<8; j++){
			grid[i][j] =0;
		}
	}

}



