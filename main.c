/******************************************************
** Program: main.c
** Author: Stanley Hale
** Date: 010/11/2022
** Description: Mainfile for processing movies and sorting them into directories/files by year.
** Input:
** Output:
******************************************************/
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <string.h> 
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PREFIX "movies_"
#define SUFFIX ".csv"

//**STRUCTS**:
struct Movie {
	char* title;
	int year;
	char* languages;
	float rating;
	struct Movie* next; 		
};


//**STRUCT
/******************************************************************
 * * ** Function: createMovie(char* currLine)
 * ** Description: Constructor for the Movie structure
 * ** Parameters: currLine
 * ** Pre-conditions: 
 * ** Post-conditions:
 * *******************************************************************
 */
struct Movie* createMovie(char* currLine) {
	struct Movie* currMovie = malloc(sizeof(struct Movie));
	
	char* saveptr;													//Using this variable for strtok_r
	
	//Grab the movie title
	char* token = strtok_r(currLine, ",", &saveptr);
	currMovie->title = calloc(strlen(token) +1, sizeof(char));		//Reserve memory for the exttracted title
	strcpy(currMovie->title, token);								//Copy the title over

	//Grab the movie year
	token = strtok_r(NULL, ",", &saveptr);
	currMovie->year = atoi(token);
	
	//Grab movie languages
	token = strtok_r(NULL, ",", &saveptr);
	currMovie->languages = calloc(strlen(token) +1, sizeof(char));
	strcpy(currMovie->languages, token);

	//Grab movie rating
	token = strtok_r(NULL, "\n", &saveptr);
	currMovie->rating = atof(token);

	//Set the next node of the list to NULL 
	currMovie->next = NULL;

	return currMovie;
}

//**STRUCT
/******************************************************************
 * * ** Function: processFile(char* filePath)
 * ** Description: create a linked list of movies from a file.
 * ** Parameters: filePath
 * ** Pre-conditions: 
 * ** Post-conditions:
 * *******************************************************************
 */
struct Movie* processFile(char* filePath) {	

	//Open the file, reading only
	FILE* movieFile = fopen(filePath, "r");
	
	char* currLine = NULL;
	size_t len = 0;
	ssize_t nread;
	char* token;

	//Create a linked list head
	struct Movie* head = NULL;
	
	//Create a linked list tail
	struct Movie* tail = NULL;

	//**Skip the first line**
	nread = getline(&currLine, &len, movieFile);

	//Read in the file line by line
	while ((nread = getline(&currLine, &len, movieFile)) != -1)
	{
		//Create a new node based of the extracted line
		struct Movie* newNode = createMovie(currLine);

		if(head == NULL) 
		{
			//This will create the first node in the list
			head = newNode;
			tail = newNode;
		}
		else
		{
			//Add the node to the end of the list
			tail->next = newNode;
			tail = newNode;
		}
	}
	free(currLine);
	fclose(movieFile);
	return head;
}


//**STRUCT 
/******************************************************************
 * * ** Function: printMovie(struct Movie* aMovie)
 * ** Description: Prints out the data for the given movie
 * ** Parameters: aMovie
 * ** Pre-conditions: 
 * ** Post-conditions:
 * *******************************************************************
 */
void printMovie(struct Movie* aMovie) {
	printf("%s, %d, %s, %.1f\n",
		aMovie->title,
		aMovie->year,
		aMovie->languages,
		aMovie->rating);
}

//**STRUCT
/******************************************************************
 * * ** Function: printMovieList(struct Movie* list)
 * ** Description: Prints the whole linked list
 * ** Parameters: list
 * ** Pre-conditions: 
 * ** Post-conditions:
 * *******************************************************************
 */
void printMovieList(struct Movie* list) {
	while(list != NULL) 
	{
		printMovie(list);
		list = list->next;
	}
}

//**STRUCT
/******************************************************************
 * * ** Function: movieDestructor(struct Movie* list)
 * ** Description: Free's memory for each movie in the list
 * ** Parameters: list
 * ** Pre-conditions: 
 * ** Post-conditions:
 * *******************************************************************
 */
void movieDestructor(struct Movie* list) {
	struct Movie* prevNode;
	
	while (list != NULL)
	{
		//Free memory allocated on the heap for this movie
		//printf("Deleting title\n");
		
		free(list->title);
		//printf("Deleting lang\n");
		free(list->languages);
		
		//
		if(prevNode == NULL)
		{
			//printf("First Node!\n");
			prevNode = list;
			list = list->next;
		}
		else
		{
		
			//printf("Deleting prevNode\n");
			//free(prevNode);
			if(list != NULL)
			{
				prevNode = list;
				list = list->next;
			}
		}
	}
}



//**FUNCTIONS:
/******************************************************************
 * * ** Function: startPrompt()
 * ** Description: Prompts user forr input and sends back the user inputted int.
 * ** Parameters: 
 * ** Pre-conditions: 
 * ** Post-conditions: int of input
 * *******************************************************************
 */
int startPrompt() {
	
	char input = '0';

	while ( !( (input <= 50) && (input >=49) ) ) 
	{
		input = '0';
		printf("1. Select file to process\n");
		printf("2. Exit the program\n");
		printf("Enter a choice 1 or 2: ");
		
		scanf("%c", &input);
		//printf("\n");
		//printf("Inputted : %d\n", input);
		
		if( !( (input <= 50) && (input >=49) ) ) 
		{
				printf("\n||Error|| Please input a number between 1 and 2!\n\n");
		}
		
		
	}
	return (input - 48);
}

/******************************************************************
 * * ** Function: processPrompt()
 * ** Description: Prompts user for input and sends back the user inputted int.
 * ** Parameters: 
 * ** Pre-conditions: 
 * ** Post-conditions: int of input
 * *******************************************************************
 */
int processPrompt() {
	char input = '0';
	while ( !( (input <= 51) && (input >=49) ) ) 
	{
		input = '0';
		printf("Which file you want to process?\n");
		printf("Enter 1 to pick the largest file\n");
		printf("Enter 2 to pick the smallest file\n");
		printf("Enter 3 to specify the name of a file\n");
		printf("Enter a choice between 1 or 3: ");
		
		scanf("%c", &input);
		//printf("\n");
		//printf("Inputted : %d\n", input);
		
		if( !( (input <= 51) && (input >=49) ) ) 
		{
				printf("\n||Error|| Please input a number between 1 and 3!\n\n");
		}
		
		
	}
	return (input - 48);
}

/******************************************************************
 * * ** Function: fileLargest()
 * ** Description: Searches the curent directory for the largest file with the prefix and suffix then processes it.
 * ** Parameters: 
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void fileLargest() {
	//**Code adapted off of "3_5_stat_example.c"
	
	// Open the current directory
	DIR* currDir = opendir(".");
	struct dirent *aDir;
	off_t size = 0;
	struct stat dirStat;
	char entryName[256];
	char* ret;
	
	
	// Go through all the entries
	while((aDir = readdir(currDir)) != NULL){
		
		//Compare the PREFIX of the directory/file
		if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){
			//Compare the SUFFIX of the directory/file
			ret = strstr(aDir->d_name, SUFFIX);						//Search the file/directory name for ".csv"
			if( (ret != NULL) && (strcmp(ret, SUFFIX) == 0) ) {		//If .csv is found and is equal to ".csv" exactly
				
				// Get meta-data for the current entry
				stat(aDir->d_name, &dirStat); 
				
				//Check if this file's size is greater than the previous highest, replace it
				if( dirStat.st_size >= size) {
					size = dirStat.st_size;
					memset(entryName, '\0', sizeof(entryName));
					strcpy(entryName, aDir->d_name);
				}
			}
		}
		
	}
	//DO stuff
	printf("Processing greatest file size found was of file ./%s with a size of %d bytes\n", entryName, size);  
	
	//Close the current directory
	closedir(currDir);
	processMovies(entryName);
	
	//char* tmp = entryName;
	//return tmp;
}

/******************************************************************
 * * ** Function: fileSmallest()
 * ** Description: Searches the curent directory for the smallest file with the prefix and suffix then processes it.
 * ** Parameters: 
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void fileSmallest() {
	//**Code adapted off of "3_5_stat_example.c"
		
	// Open the current directory
	DIR* currDir = opendir(".");
	struct dirent *aDir;
	off_t size = 0;
	struct stat dirStat;
	char entryName[256];
	char* ret;
	
	
	// Go through all the entries
	while((aDir = readdir(currDir)) != NULL){
		
		//Compare the PREFIX of the directory/file
		if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){
			//Compare the SUFFIX of the directory/file
			ret = strstr(aDir->d_name, SUFFIX);						//Search the file/directory name for ".csv"
			if( (ret != NULL) && (strcmp(ret, SUFFIX) == 0) ) {		//If .csv is found and is equal to ".csv" exactly
				
				// Get meta-data for the current entry
				stat(aDir->d_name, &dirStat); 
				
				//Check if this file's size is smaller than the previous highest, replace it (Or if the size is not initialized)
				if( dirStat.st_size <= size || size == 0) {
					size = dirStat.st_size;
					memset(entryName, '\0', sizeof(entryName));
					strcpy(entryName, aDir->d_name);
				}
			}
		}
		
	}
	//DO stuff
	printf("Processing smallest file size found was of file ./%s with a size of %d bytes\n", entryName, size);  
	
	//Close the current directory
	closedir(currDir);
	processMovies(entryName);
	
	//char* tmp = entryName;
	//return tmp;
}

/******************************************************************
 * * ** Function: fileExist(int* again)
 * ** Description: Searches the curent directory for the inputted file with the prefix and suffix then processes it.
 * ** Parameters: int* again
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void fileExist(int* again) {
	//**Code adapted off of "3_5_stat_example.c"
	
	// Open the current directory
	DIR* currDir = opendir(".");
	struct dirent *aDir;
	struct stat dirStat;
	char entryName[256];
	memset(entryName, '\0', sizeof(entryName));
	int count = 0;
	
	//Get input of a string from the user.
	printf("Enter the complete file name: ");
	char str_input[20];
	memset(str_input, '\0', sizeof(str_input));

	scanf("%s", str_input);
	
	
	// Go through all the entries
	while((aDir = readdir(currDir)) != NULL){
		if( strcmp(str_input, aDir->d_name) == 0) {
			memset(entryName, '\0', sizeof(entryName));
			strcpy(entryName, aDir->d_name);
			count=1;
			break; 													//If the file was found, stop searching (wastes processing time)
		}
	}
	
	//Check if the file was found.
	if(count == 0) {
		printf("\n||Error|| The file %s was not found. Try again\n\n", str_input);
		*again = 1;
		return;
	}
	else {
		printf("Processing chosen file ./%s\n", entryName); 
		*again = 0;
	}
	 
	
	//Close the current directory
	closedir(currDir);
	processMovies(entryName);
	
	//char* tmp = entryName;
	//return tmp;
}


/******************************************************************
 * * ** Function: genRandNum()
 * ** Description: Generates and returns a random number between 0 and 90000
 * ** Parameters: 
 * ** Pre-conditions: 
 * ** Post-conditions: long random number
 * *******************************************************************
 */
long genRandNum() {
	return ( (rand() %99999)+1 );
} 

/******************************************************************
 * * ** Function: processMovies(char* target_file)
 * ** Description: Creates a linked list and reads in movies for the targe_file. Then create s adirectory and creates text files for each year and stores the coresponding movie titles in them.
 * ** Parameters: char* target_file
 * ** Pre-conditions: 
 * ** Post-conditions: Creates a new directory with text files of the years of movies.  
 * *******************************************************************
 */
void processMovies(char* target_file) {
	int file_descriptor;
	ssize_t nwritten;
	int tempNum = 0;
	
	//printf("%s\n", target_file);
	struct Movie* list = processFile(target_file);
	//printMovieList(list);
	
	//Create a new directory with random number in format "onid.movies.random"
	long randNum = genRandNum();
	
	char dirName[] = "halesta.movies.";
	
	//Convert random number into a string
	char str_randNum[10];
	sprintf(str_randNum, "%lld", randNum);
	//printf("%s\n", str_randNum);
	
	//Cat the two strings into dirName
	strcat(dirName, str_randNum);
	
	//Create the directory with: rwx, r-x, ---
	int ret = mkdir(dirName, 0750);

    if (ret == 0) {
        printf("Created directory with name %s\n", dirName);
	}
    else {
        printf("||ERROR|| Unable to create directory %s\n", dirName);
	}
	
	//Change directory to the newly created one
	chdir(dirName);
	char extension[] = ".txt";
	char fileName[10];
	//const char endline = '\n';
	char* endline = "\n";
	
	
		
	//Go through the list and create/append a file for each year
	while(list != NULL)
	{
		//printf("In while loop!\n");
		//Check if we've gotten to the end of the list
		if( list == NULL) {
			break;
		}
			
		//Convert year(int) into a string
		memset(fileName, '\0', sizeof(fileName));
		sprintf(fileName, "%lld", list->year);
			
		//cat .txt to the end of the fileName
		strcat(fileName, extension);
		//printf("fileName: %s\n", fileName);
			
		//Create a file (with rw-, r--, ---) or open if it exists and append
		file_descriptor = open(fileName, O_WRONLY | O_CREAT | O_APPEND, 0640); 
		//Check to see if opening the file succeeded.
		if (file_descriptor == -1){
			printf("open() failed on \"%s\"\n", fileName);
			perror("Error");
			exit(1);
		}
				
				
		//Convert the title into one with a newline
		//char newTitle[sizeof(list->title)+2];
		//memset(newTitle, '\0', sizeof(newTitle));
		//strcpy(newTitle, list->title);
		//newTitle[sizeof(newTitle) - 1] = '\n';
		//newTitle[sizeof(newTitle)] = '\0';
		 
		
		//Write to file
		nwritten = write(file_descriptor, list->title, strlen(list->title) * sizeof(char));
		nwritten = write(file_descriptor, endline, strlen(endline) * sizeof(char));
		close(file_descriptor);
			
		
		list = list->next;
	}
	
}

/******************************************************************
 * * ** Function: int main()
 * ** Description: Processes movie files and creates a new directory with the years of each movie.
 * ** Parameters:
 * ** Pre-conditions: 
 * ** Post-conditions: Creates a new directory with text files of the years of movies.  
 * *******************************************************************
 */
int main() {
	
	srand(time(0));													//Seed random to time
	int again1 = 1;
	int again2 = 1;
	char* targetFile;
	int start;
	int proc;
		
	//long randNum = genRandNum();
	//printf("Rand Number = %d\n", randNum);
	//processMovies("movies_sample_1.csv");
	
	//Prompt user for input.
	//while(again1 == 1) {
		again2 = 1;
		start = startPrompt();
		if(start == 2) {
				return 0; 											//If the user inputted '2', then exit tthe program
		}
	
		//Else we prompt 3 other choices.
		while( again2 == 1) {
			proc = processPrompt();
			if(proc == 1) {
				/*targetFile =*/ fileLargest();
				//processMovies(targetFile);
				again2 = 0;
				//printf("%s\n", targetFile);
			}
			else if (proc == 2) {
				/*targetFile =*/ fileSmallest();
				//processMovies(targetFile);
				again2 = 0;
				//printf("%s\n", targetFile);
			}
			else if (proc ==3)
			{
				/*targetFile =*/ fileExist(&again2);
				//processMovies(targetFile);
				//printf("%s\n", targetFile);
			}
		}
	//}
	
	return 0;
}