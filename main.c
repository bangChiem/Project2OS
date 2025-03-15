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
	for(int i = 0; i < 9; i++){
		if(correctPuzzle != 1){
			correctPuzzle = checker[i] == 1;
		}		
	}
	pthread_exit(0);
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
	clock_t begin = clock();
	int n1 = fork();
	int n2 = fork();

	// create shared memory
	const int SIZE = 4096;
	const char* shm_name = "shared_memory";
	int fd;
	int *shm_worker_threads;
	fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
	ftruncate(fd,SIZE);
	shm_worker_threads = (int *)mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	

	if (n1 > 0 && n2 > 0){
		wait(NULL);
		printf("parent\n");

		// load file to log output
		FILE *res;
		res = fopen("results.txt", "a+");

		// get completion time
		clock_t end = clock();
		double duration = (double) (end - begin) / CLOCKS_PER_SEC;

		// check validation status for worker threads
		int puzzleIsTrue = 1;
		for (int i = 0; i < 2; i++)
		{
			if (shm_worker_threads[i] == 0){
				puzzleIsTrue = 0;
			}
		}
		
		if (puzzleIsTrue){		
			printf("SOLUTION: YES in %f\n", duration);
			fprintf(res, "%f\n", duration);
		}
		else{
			printf("SOLUTION: NO in %f\n", duration);
			fprintf(res, "%f\n", duration);
		}
	}

	// check rows in process 1; index 0 in shm_worker_threads marks validation value for rows
	else if (n1 == 0 && n2 > 0){
		printf("process 1\n");
		for (int i = 0; i < 9; i++)
		{	
			if (check_row(i) == 1){
				shm_worker_threads[0] = 1;
			}
			else{
				shm_worker_threads[0] = 0;
			}
		}
	}

	// check cols in process 1; index 1 in shm_worker_threads marks validation value for cols
	else if (n1 > 0 && n2 == 0){
		printf("process 2\n");
		for (int i = 0; i < 9; i++)
		{	
			if (check_col(i) == 1){
				shm_worker_threads[1] = 1;
			}
			else{
				shm_worker_threads[1] = 0;
			}
		}
	}

	else{
		printf("process 3\n");
	}
}

// call appropiate amount of worker threads according to option
// use each thread to check a certain part of the puzzle for if it is correct
void call_threads(int option){
	if (option == 3){
		handleOption3();
	}

	if (option == 2 || option == 1){
		pthread_t pid[9];
		pthread_t square_params[9];
		for(int i = 0; i < 9; i++){
			square_params[i] = i;
			pthread_create(&pid[i], NULL, square, &square_params[i]);
		}

		for(int i = 0; i < 9; i++){
			pthread_join(pid[i], NULL);
		}
	}

	int cols[9];
	int rows[9];
	if (option == 1){
		pthread_t row_t;
		pthread_t col_t;

		pthread_create(&row_t, NULL, option1_row, NULL);
		pthread_create(&col_t, NULL, option1_col, NULL);

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
			pthread_create(&rows_t[i], NULL, option2_row, &rows[i]);
			pthread_create(&cols_t[i], NULL, option2_column, &cols[i]);
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
		}
		return 1;
	}
	else if (option == 1){
		if (rowWorkerThreads[0] == 0 || colWorkerThreads[0] == 0){
			return 0;
		}
		else if (rowWorkerThreads[0] == 1 && colWorkerThreads[0] == 1){
			return 1;
		}
		else{
			return -1;
		}	
	}
}

int main(int argc, char** argv){
	clock_t begin = clock();
	int option = atoi(argv[1]);
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
	call_threads(option); //driver
	double duration = (double) (end - begin) / CLOCKS_PER_SEC;

	FILE *res;
	res = fopen("Option1Yes.txt", "a+");

	//print puzzle result
	if (option != 3){ // exclude if option 3 since spanwed process will incorrectly print 3 result statements
		if (check_worker_array(option) == 1){
			printf("SOLUTION: YES in %f\n", duration);
			fprintf(res, "%f\n", duration);
	}
		else if (check_worker_array(option) == 0){
			printf("SOLUTION: NO in %f\n", duration);
			fprintf(res, "%f\n", duration);
		}
		else{
			printf("ERROR\n");
		}
	}

	return 0;
}
