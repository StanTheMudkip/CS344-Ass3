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
		int argc;															//Keeps count of the number of arguments
		char* argv[420];													//Set max of arguments to 420 characters (includes the actual command along side arguments)
		int BackGround;														//Keeps track if this is a background proccess?
};




//**FUNCTIONS:
void printStr(char* str) {
	printf("%s\n", str);
	fflush(stdout);
}

void printCommand(command cmd) {
	printf("Argc: %d, Argv: ", cmd.argc);
	for ( int i = 0; i < cmd.argc; i++) 
	{
		printf("%s ", cmd.argv[i]);
		fflush(stdout);
	}
	printf("\n");
}


void prompt(command* cmd) {
	char* token;
	char* saveptr;															//Using this variable for strtok_r
	
	char str_input[420];
	memset(str_input, '\0', sizeof(str_input));								//Clears the string memory to only NULL
	printf(": ");															//Character to denote that we are ready for a new command.
	fflush(stdout);
	
	//Read in a line from the user
	fgets(str_input, 419, stdin);											//take 419 just to make sure we have enough room in the string for a NULL terminator
	//printf("The command you input is: %s\n", str_input);
	//fflush(stdout);
	
	//check if we have filled the string and that it is greater than 1
	if( (str_input[strlen(str_input)-1] == '\n') && (strlen(str_input) > 1) )
	{
		str_input[strlen(str_input)-1] = '\0';								//Replace the newline with a null terminator so that we can tokenize this string and parse it.
	}
	//printf("The command you input w/out a (natural) '\\n' is: %s\n", str_input);
	//fflush(stdout);
	
	
	//Parse the input
	
	token = strtok_r(str_input, " ", &saveptr); 							//Grabs the first argument (the actual command) 
	//printStr(token);
	
	while(token != NULL) 
	{
		//Grab and store the data in the cmd struct
		cmd->argv[cmd->argc] = malloc( sizeof(strlen(token)) ); 				//allocate memory for the new command
		memset( cmd->argv[cmd->argc], '\0', sizeof(cmd->argv[cmd->argc]) );		//Clears the allocated string memory for copying.
		strcpy(cmd->argv[cmd->argc], token);									
		cmd->argc += 1;														//increment the number of arguments
		
		token = token = strtok_r(NULL, " ", &saveptr); 						//Grab the next argument (if it doesnt exist it will be NULL)
		
		/*Prints each token that is aquired.
		if(token != NULL) 
		{
			printStr(token);
		}
		*/
	}
	
	//Set the last cmd argument to NULL for when we call the process in exec
	cmd->argv[cmd->argc] = NULL;
	
	//Check if this needs to be set as a background process? ( '&' the background notifier will always be the last argument of each command besides NULL.)
	
}


int main() {
	command cmd;															//Create an instance of my command struct
	cmd.argc = 0;															//Set the unitialized argument count to 0;
	
	
	//Prompt user for input
	prompt(&cmd);
	//printCommand(cmd);
	
	//check if the command (first argument) is a newline
	if( strcmp(cmd.argv[0], "\n") )
	{
		//check if the command (first charcter of the first argument) is a comment!
		if(cmd.argv[0][0] != '#')
		{
			//This is where we can check for our versions of cd, exit, and status
			if( strcmp(cmd.argv[0], "exit") == 0)								//If we enter exit, we just exit the shell (aka this program/process)
			{
				return 0;
			}
			else if( strcmp(cmd.argv[0], "cd") == 0 )
			{
				printf("Changing Directory\n");
				fflush(stdout);
			}
			else if( strcmp(cmd.argv[0], "status") == 0 )
			{
				printf("Showing status\n");
				fflush(stdout);
			}
			//Then handle other commands?
			
		}
	}
	
	
	return 0;
}