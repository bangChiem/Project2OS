#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

//gloabal vairable to fill with input
int puzzle[9][9];

//global flag variable to declare if puzzle is correct or wrong
int correctPuzzle = 1;

void * square(void* param){
	int squareNum = *(int*)param;
	int checker[9] = {0};
 

	//Load in the square
	for(int i = 0; i<3; i++){
		for(int j = 0; j<3; j++){
			int pRow = squareNum/3 *3 + i;
			int pCol = squareNum%3 *3 + j;
			int val = puzzle[pRow][pCol];
			checker[val - 1]++;
		}
	}

	//validates puzzle
	for(int i = 0; i < 9; i++){
		if(correctPuzzle != 1){
			correctPuzzle = checker[i] == 1;
		}		
	}
	pthread_exit(0);
}


//thread algorithms to validate
void *column(void* param){

	int checker[9];

	// initialize checker arr to all 0
	for (int i = 0; i < 9; i++)
	{
		checker[i] = 0;
	}

	// go each val through each val in column and fill in checker array
	int column = *(int*)param - 1;
	int temp;
	for (int i = 0; i < 9; i++){
		temp = puzzle[i][column];
		if (temp == 0){
			continue;;
		}
		checker[temp - 1] = 1;
	}

	// if all values in checker are 1 then column has all digits 1-9
	for (int i = 0; i < 9; i++)
	{
		if (checker[i] == 0){
			// mark global validator value false
			correctPuzzle = 0;
		}
	}
}

void *row(void* param){
	int checker[9];

	for (int i = 0; i < 9; i++)	// initialize checker arr to all 0
	{
		checker[i] = 0;
	}

	// go each val through each val in row and fill in checker array
	int row = *(int*)param - 1;
	int temp;
	for (int i = 0; i < 9; i++){
		temp = puzzle[row][i];
		if (temp == 0){
			continue;;
		}
		checker[temp - 1] = 1;
	}

	// if all values in checker are 1 then column has all digits 1-9
	for (int i = 0; i < 9; i++)
	{
		if (checker[i] == 0){
			// mark global validator value false
			correctPuzzle = 0;
		}
	}
}
 
void call_threads(int option){
	pthread_t pid[9];
	pthread_t square_params[9];
	for(int i = 0; i < 9; i++){
		square_params[i] = i;
		pthread_create(&pid[i], NULL, square, &square_params[i]);
	}

	for(int i = 0; i < 9; i++){
		pthread_join(pid[i], NULL);
	}


	int cols[9];
	int rows[9];
	if (option == 1){
		pthread_t row_t;
		pthread_t col_t;

		for (int i = 0; i < 9; i++)
		{
			cols[i] = i + 1;
			rows[i] = i + 1;
			pthread_create(&row_t, NULL, row, &rows[i]);
			pthread_create(&col_t, NULL, column, &cols[i]);
		}	

		pthread_join(row_t, NULL);
		pthread_join(col_t, NULL);
	}

	else if (option == 2){
		pthread_t rows_t[9];
		pthread_t cols_t[9];

		// create threads to cols
		for (int i = 0; i < 9; i++)
		{
			cols[i] = i + 1;
			rows[i] = i + 1;
			pthread_create(&rows_t[i], NULL, row, &rows[i]);
			pthread_create(&cols_t[i], NULL, column, &cols[i]);
		}

		// join threads
		for (int i = 0; i < 9; i++)
		{
			pthread_join(rows_t[i], NULL);
			pthread_join(cols_t[i], NULL);
		}
	}
	
}

int main(int argc, char** argv){
	clock_t begin = clock();
	int option = 0;
	FILE *fptr = NULL;

	if(argc != 2){
		fprintf(stderr, "Bad Arguments, please select 1 option\n");
		exit(1);
	}

	//Open and read input.txt
	fptr = fopen("input.txt", "r");
	if(fptr == NULL)fprintf(stderr, "Error opening input.txt\n");
	for(int i = 0; i < 9; i++){
		for (int j = 0; j < 9; j++){
			fscanf(fptr, "%d ", &puzzle[i][j]);
			printf("%d ", puzzle[i][j]);
		}
		printf("\n");
	}

	char* SOLUTION;
	clock_t end = clock();
	call_threads(2); //driver
	double duration = (double) (end - begin) / CLOCKS_PER_SEC;

	FILE *res;
	res = fopen("results.txt", "a+");

	if (correctPuzzle){
		printf("SOLUTION: YES in %f\n", duration);
		fprintf(res, "%f\n", duration);
	}
	else{
		printf("SOLUTION: NO in %f\n", duration);
		fprintf(res, "%f\n", duration);
	}
	return 0;
}
