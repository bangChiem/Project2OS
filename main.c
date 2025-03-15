#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/mman.h>
#include<fcntl.h>


//gloabal vairable to fill with input
int puzzle[9][9];

// confirmation array
int colWorkerThreads[9];
int rowWorkerThreads[9];
int squareWorkerThreads[9];

// global flag variable to declare if puzzle is correct or wrong
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
	//return true or false depending on square validation
	void *retval;
	retval = malloc(sizeof(int));
	*(int*)retval = 1;
	for(int i = 0; i < 9; i++){
		if(checker[i] != 1){
			*(int*)retval = 0;
		}		
	}
	pthread_exit(retval);
}

int check_row(int row){
	int checker[9];

	for (int i = 0; i < 9; i++)	// initialize checker arr to all 0
	{
		checker[i] = 0;
	}

	// go each val through each val in row and fill in checker array
	int temp;
	for (int i = 0; i < 9; i++){
		temp = puzzle[row][i];
		if (temp == 0){
			continue;
		}
		checker[temp - 1] = 1;
	}

	// if all values in checker are 1 then row has all digits 1-9
	for (int i = 0; i < 9; i++)
	{
		if (checker[i] == 0){
			return 0;
		}
	}
	return 1;
}

int check_col(int col){
	int checker[9];

	for (int i = 0; i < 9; i++)	// initialize checker arr to all 0
	{
		checker[i] = 0;
	}

	// go each val through each val in row and fill in checker array
	int temp;
	for (int i = 0; i < 9; i++){
		temp = puzzle[i][col];
		if (temp == 0){
			continue;
		}
		checker[temp - 1] = 1;
	}

	// if all values in checker are 1 then column has all digits 1-9
	for (int i = 0; i < 9; i++)
	{
		if (checker[i] == 0){
			return 0;
		}
	}
	return 1;
}

void *option2_row(void* param){
	int row = *(int*)param - 1;
	if (check_row(row) == 0){
		rowWorkerThreads[row] = 0;
	} else{
		rowWorkerThreads[row] = 1;
	}
	pthread_exit(0);
}

//thread algorithms to validate
void *option2_column(void* param){
	int column = *(int*)param - 1;
	if (check_col(column) == 0){
		colWorkerThreads[column] = 0;
	}
	else{
		colWorkerThreads[column] = 1;
	}
	pthread_exit(0);
}

// read all 9 rows and validate update workerthreads arr with validation status
void *option1_row(){
	rowWorkerThreads[0] = 1;
	for (int i = 0; i < 9; i++)
	{	
		if (check_row(i) == 0){
			rowWorkerThreads[0] = 0;
		}
	}
}

// read all 9 cols and validate update workerthreads arr with validation status
void *option1_col(){
	colWorkerThreads[0] = 1;
	for (int i = 0; i < 9; i++)
	{
		if (check_col(i) == 0){
			colWorkerThreads[0] = 0;
		}
	}
}
 
// create three processes one to check rows, cols, grids
void handleOption3(){	
	// create shared memory
	const int SIZE = 4096;
	const char* shm_name = "shared_memory";
	int fd;
	int *shmPtr;
	fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
	ftruncate(fd,SIZE);
	shmPtr = (int*) mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	
	
	pid_t n1 = -1, n2= -1, n3 = -1;
	n1 = fork();	

	if(n1<0){
		printf("Forking Error\n");
		exit(1);
	}

	if(n1>0){ //if parent
		n2 = fork();
		if(n2<0){
		printf("Forking Error\n");
			exit(1);
		}
		if(n2>0){	//if parent
			n3 = fork();
			if(n3<0){
				printf("Forking Error\n");
				exit(1);
			}
		}
	}


	if(n1 == 0){//Child Process 1 will check rows
	//	printf("process 1\n");
		int rowCorrect = 1;
		for (int i = 0; i < 9; i++)
			if(check_row(i) == 0)
				rowCorrect = 0;
		
		shmPtr[0] = rowCorrect;
		exit(0); //exit process
	}
	else if(n2 == 0){//Child Process 2 will check columns
	//	printf("process 2\n");
		int colCorrect = 1;
		for (int i = 0; i < 9; i++)
		{	
			if (check_col(i) == 0){
				colCorrect = 0;

			}
		}
		shmPtr[1] = colCorrect;
		exit(0);//exit process
	}
	else if(n3 == 0){//Child Process 3 will check squares
	//	printf("Process 3\n");
			
		pthread_t pid[9];
		int square_params[9];
		void *retvals[9];
		int squareCorrect = 1; //new bool
		for(int i = 0; i < 9; i++){
			square_params[i] = i;
			pthread_create(&pid[i], NULL, square, &square_params[i]);
		}
		for(int i = 0; i < 9; i++){
			pthread_join(pid[i], &retvals[i]);
			if(*(int*)retvals[i] == 0) //change bool if false
				squareCorrect = 0;
			free(retvals[i]);
		}

		shmPtr[2] = squareCorrect;
		exit(0);
	}
	//Only Parent should exist past here
	wait(NULL);
	wait(NULL);
	wait(NULL);
	printf("Children Complete\n");
	//Debug statment printf("TEST \'%d%d%d'\n", shmPtr[0],shmPtr[1],shmPtr[2]);
	
	correctPuzzle = shmPtr[0] && shmPtr[1] && shmPtr[2];
	shm_unlink(shm_name);
}

// call appropiate amount of worker threads according to option
// use each thread to check a certain part of the puzzle for if it is correct
void call_threads(int option){
	if (option == 3){
		handleOption3();
	}

	if (option == 2 || option == 1){
		pthread_t pid[9];
		int square_params[9];
		void *retvals[9];
		for(int i = 0; i < 9; i++){
			square_params[i] = i;
			pthread_create(&pid[i], NULL, square, &square_params[i]);
		}
		for(int i = 0; i < 9; i++){
			pthread_join(pid[i], &retvals[i]);
			squareWorkerThreads[i] = *(int*)retvals[i];
			free(retvals[i]);
		}
	}

	if (option == 1){
		pthread_t row_t;
		pthread_t col_t;

		pthread_create(&row_t, NULL, option1_row, NULL);
		pthread_create(&col_t, NULL, option1_col, NULL);

		pthread_join(row_t, NULL);
		pthread_join(col_t, NULL);
	}

	else if (option == 2){
		int cols_params[9];
		int rows_params[9];
		pthread_t rows_t[9];
		pthread_t cols_t[9];

		// create threads to cols
		for (int i = 0; i < 9; i++)
		{
			cols_params[i] = i + 1;
			rows_params[i] = i + 1;
			pthread_create(&rows_t[i], NULL, option2_row, &rows_params[i]);
			pthread_create(&cols_t[i], NULL, option2_column, &cols_params[i]);
		}

		// join threads
		for (int i = 0; i < 9; i++)
		{
			pthread_join(rows_t[i], NULL);
			pthread_join(cols_t[i], NULL);
		}
	}
}

// debug function
void print_worker_array(int arr[]){
	printf("worker array: ");
	for (int i = 0; i < 1; i++)
	{
		printf("%d ", arr[i]);
	}
	printf("\n");
	
}

// if all worker array marked there fields as correct return true / else return false
int check_worker_array(int option){
	if (option == 2){
		for (int i = 0; i < 9; i++)
		{
			if (rowWorkerThreads[i] == 0){
				return 0;
			}
			if (colWorkerThreads[i] == 0){
				return 0;
			}
			if (squareWorkerThreads[i] == 0){
				return 0;
			}
		}
		return 1;
	}
	else if (option == 1){

		//checks square
		for(int i = 0; i < 9; i++)
			if(squareWorkerThreads[i] == 0)
				return 0;

		//checks row and column
		if (rowWorkerThreads[0] == 0 || colWorkerThreads[0] == 0){
			return 0;
		}
		else if (rowWorkerThreads[0] == 1 && colWorkerThreads[0] == 1){
			return 1;
		}
	}
}

int main(int argc, char** argv){
	clock_t begin = clock();
	FILE *fptr = NULL;

	if(argc != 2){
		fprintf(stderr, "Bad Arguments, please select 1 option\n");
		exit(1);
	}

	int option = atoi(argv[1]);

	char const * const input_name = "input.txt";

	//Open and read input.txt
	fptr = fopen(input_name, "r");
	if(fptr == NULL)fprintf(stderr, "Error opening input.txt\n");
	printf("BOARD STATE IN %s:\n", input_name);
	for(int i = 0; i < 9; i++){
		for (int j = 0; j < 9; j++){
			fscanf(fptr, "%d ", &puzzle[i][j]);
			printf("%d ", puzzle[i][j]);
		}
		printf("\n");
	}

	char* SOLUTION;
	//TODO Dearest Bang why is the clock end before call threads?
	clock_t end = clock();
	call_threads(option); //driver
	double duration = (double) (end - begin) / CLOCKS_PER_SEC;

	// start logging
	FILE *res;
	res = fopen("Option1Yes.txt", "a+");

	if (option != 3){
		correctPuzzle = check_worker_array(option);
	}

	//print puzzle result
	if (correctPuzzle){
		printf("SOLUTION: YES in %f\n", duration);
		fprintf(res, "%f\n", duration);
	}
	else if (!correctPuzzle){
		printf("SOLUTION: NO in %f\n", duration);
		fprintf(res, "%f\n", duration);
	}
	return 0;
}
