#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

//thread algorithms to validate
void column(void* param);
void row(void* param);
void square(void* param);

//gloabl vairable to fill with input
int puzzle[9][9];

int main(int argc, char** argv){
	int option = 0;
	FILE *fptr = NULL;

	if(argc != 2){
		fprintf(stderr, "Bad Arguments, please select 1 option\n");
	}
	//Open and read input.txt
	fptr = fopen("input.txt", "r");
	if(fptr == NULL)fprintf(stderr, "Error opening input.txt\n");
	for(int i = 0; i < 9; i++){
		for (int j = 0; j < 9; j++){
			fscanf(fptr, "%d ", &puzzle[i][j]);
		}
	}	

	//Create threads for rows
	//Create threads for columns
	//Create threads for squares


	//Join Threads


	fclose(fptr);
	return 0;
}
