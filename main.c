/******************************************************
** Program: main.c
** Author: Stanley Hale
** Date: 010/23/2022
** Description: 
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
#include <sys/wait.h>
#include <signal.h>

//**STRUCTS:
//defines command as a standard type for use
typedef struct cmmd command;
//Structure of commands that will be used
struct cmmd {
		int argc;										//Keeps count of the number of arguments
		char* argv[420];								//Set max of arguments to 420 characters (includes the actual command)
		int BackGround;									//Keeps track if this is a background proccess?
};

void prompt() {
	char str_input[420];
	memset(str_input, '\0', sizeof(str_input));			//Clears the string memory to only NULL
	printf(": ");										//Character to denote that we are ready for a new command.
	
	//Read in a line from the user
	fgets(str_input, 419, stdin);						//take 419 just to make sure we have enough room in the string for a NULL terminator
	//printf("The command you input is: %s\n", str_input);
	//fflush(stdout);
	
	//check if we have filled the string and that it is greater than 1
	if( (str_input[strlen(str_input)-1] == '\n') && (strlen(str_input) > 1) )
	{
		str_input[strlen(str_input)-1] = '\0';			//Replace the newline with a null terminator so that we can tokenize this string and parse it.
	}
	//printf("The command you input w/out a (natural) '\\n' is: %s\n", str_input);
	/fflush(stdout);
	
	
}

//**FUNCTIONS:
int main() {
	
	//Prompt user for input
	prompt();
	
	
	return 0;
}