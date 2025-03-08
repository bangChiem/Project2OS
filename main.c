#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

//thread algorithms to validate
void column(void* param){

	// used to keep track of if digit 1-9 is in column
	// ex. if checker[0] is 1 then 1 is in column...
	// if checker[3] is 0 then 4 is NOT in column
	int checker[9];

	// initialize checker arr to all 0
	for (int i = 0; i < 9; i++)
	{
		checker[i] = 0;
	}

	// go each val through each val in column and fill in checker array
	int column = *(int*)param;
	int temp;
	for (int i = 0; i < 9; i++){
		temp = puzzle[i][column];
		checker[temp - 1] = 1;
	}

	// if all values in checker are 1 then column has all digits 1-9
	for (int i = 0; i < 9; i++)
	{
		if (checker[i] == 0){
			// mark global validator value false
			correct_puzzle = 0;
		}
	}

}
void row(void* param){
	int row = *(int*)param;
}

void square(void* param){
	int square = *(int*)param;
}


//gloabal vairable to fill with input
int puzzle[9][9];

//global flag variable to declare if puzzle is correct or wrong
int correct_puzzle = 1;

int main(int argc, char** argv){


	//Open and read file form argv[1]

	//Create threads for rows
	//Create threads for columns
	//Create threads for squares


	//Join Threads

	return 0;
}
