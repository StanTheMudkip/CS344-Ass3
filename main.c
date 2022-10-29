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
		int argc;																			//Keeps count of the number of arguments
		char* argv[512];																	//Set max of arguments to (**MAX NEEDS TO BE 512 ARGS**) characters (includes the actual command along side arguments)
		int backGround;																		//Keeps track if this is a background proccess? (bool)
		int input;																			//Wether or not we need to redirect input (bool)
		int iIdx;																			//Stores the index of the input operator
		int output;																			//Wether or not we need to redirect input (bool)
		int oIdx;																			//Stores the index of the output operator
};

//Destructor
void freeCommand(command* cmd)
{
	//Go through all the arguments starting from the last argument.
	while(cmd->argc != 0)														
	{
		//printf("In loop\n");
		//fflush(stdout);
		
		if (!cmd->argv[cmd->argc])															//If the argument exists, free it
		{
			//printf("Freeing\n");
			//fflush(stdout);
			
			free(cmd->argv[cmd->argc]);
			//Decrement the argument counter
		}
		cmd->argc -= 1;
	}
	
	//Reset the allocated memory to NULL
	memset(cmd->argv, '\0', sizeof(cmd->argv));
}



typedef struct state status;
struct state {
	int status;																				//Keeps track of the exit status of a previous command/program 
	int signal;																				//Keeps track of the termination signal of the previous command/program
};

void printmyStatus(status stat) {
	//Check if there a termination signal was recieved with the last process
	if( stat.signal != 0 )
	{
		//If a signal was termination recieved from the last process, print out the singal number.
		printf("Terminated by signal: %d\n", stat.status);
		fflush(stdout);
	}
	else 
	{
		//If no signal was recieved print out the exit status
		printf("Exit status of: %d\n", stat.status);
		fflush(stdout);
	}
}


//**FUNCTIONS:
void printStr(char* str) {
	printf("%s\n", str);
	fflush(stdout);
}

void printCommand(command cmd) {
	printf("Argc: %d, Argv: ", cmd.argc);
	fflush(stdout);
	for ( int i = 0; i < cmd.argc; i++) 
	{
		printf("%s ", cmd.argv[i]);
		fflush(stdout);
	}
	printf("\n");
	fflush(stdout);
}

void prompt(command* cmd) {
	//NEED TO RESET THE COMMAND CMD SO THAT IT WONT JUST CONTAIN THE PREVIOUS COMMAND'S ARGUMENTS/VALUES AND STUFF AND TO NOT LEAK MEMORY!!!!!!!!
	freeCommand(cmd);
	
	char* token;
	char* saveptr;																			//Using this variable for strtok_r
	
	char str_input[2049];																	//Set maximum amount of characters that we can take in to 2048 (excluding null character?)
	memset(str_input, '\0', sizeof(str_input));												//Clears the string memory to only NULL
	printf(": ");																			//Character to denote that we are ready for a new command.
	fflush(stdout);
	
	//Read in a line from the user
	fgets(str_input, 2048, stdin);															//take 2048 just to make sure we have enough room in the string for a NULL terminator
	//printf("The command you input is: %s\n", str_input);
	//fflush(stdout);
	
	//check if we have filled the string and that it is greater than 1
	if( (str_input[strlen(str_input)-1] == '\n') && (strlen(str_input) > 1) )
	{
		str_input[strlen(str_input)-1] = '\0';												//Replace the newline with a null terminator so that we can tokenize this string and parse it.
	}
	//printf("The command you input w/out a (natural) '\\n' is: %s\n", str_input);
	//fflush(stdout);
	
	
	//Parse the input
	
	token = strtok_r(str_input, " ", &saveptr); 											//Grabs the first argument (the actual command) 
	//printStr(token);
	
	
	while(token != NULL) 
	{
		//Grab and store the data in the cmd struct
		cmd->argv[cmd->argc] = malloc( sizeof(strlen(token)) ); 							//allocate memory for the new command
		memset( cmd->argv[cmd->argc], '\0', sizeof(cmd->argv[cmd->argc]) );					//Clears the allocated string memory for copying.
		strcpy(cmd->argv[cmd->argc], token);									
		cmd->argc += 1;																		//increment the number of arguments
		
		token = token = strtok_r(NULL, " ", &saveptr); 										//Grab the next argument (if it doesnt exist it will be NULL)
		
		/*Prints each token that is aquired.
		if(token != NULL) 
		{
			printStr(token);
		}
		*/
	}
	

	//Set the last cmd argument to NULL for if/when we call the process in an exec
	cmd->argv[cmd->argc] = NULL;
	
	
	//Check if we need to do any redirection
	//Reset redirection variables
	cmd->output = 0;
	cmd->oIdx == 0;
	cmd->input = 0;
	cmd->iIdx = 0;
	
	//Go through all of the arguments, just need to identify if the arguments exist and where this argument is
	for(int i = 0; i < cmd->argc; i++)
	{
		//Actually check if the current argument is an alligator ( < or > )
		if( strcmp(cmd->argv[i], "<") == 0 )
		{
			//Means we are redirecting input
			cmd->input = 1;																	//Set the input bool to true
			cmd->iIdx = i;																	//Store the current index of the redirect operator
		}
		if( strcmp(cmd->argv[i], ">") == 0 )
		{
			//Means we are redirecting output
			cmd->output = 1;																//Set the input bool to true
			cmd->oIdx = i;																	//Store the current index of the redirect operator
		}
	}
	
	//Check if this needs to be set as a background process? ( '&' the background notifier will always be the last argument of each command besides NULL.)
	
}

void myCD(command* cmd) {
	//Check if we actually have a second argument to cd
	if(cmd->argv[1] != NULL)
	{
		//Check if the directory exists!
		DIR* targetDir = opendir(cmd->argv[1]);												//Attempt to open the targeted directory/path
		if(targetDir != NULL) 
		{
			//The directory exists and change to the targeted directory
			//printf("Changing to Directory: %s\n", cmd->argv[1] );
			//fflush(stdout);
			
			chdir(cmd->argv[1]);
			closedir(targetDir);															//Close the directory since we know it exists now.
			return;
		}
		else 
		{
			//If the directory wasn't found we let the user know
			//printf("cd: %s: No such file or directory\n", cmd->argv[1]);
			//fflush(stdout);
			return;
		}
	}
	//If no argument to cd, just change to home directory using env variable.
	else 
	{
		//printf("Changing to HOME Directory: %s\n", getenv("HOME") );
		//fflush(stdout);
		
		DIR* targetDir = opendir( getenv("HOME") );											//Grabs the path of the HOME env and opens the directory.
		chdir( getenv("HOME") );															//Change the current working directory to that HOME path.
		closedir(targetDir);																//Don't need to keep the directory open...
		return;
	}
}

void handOffExec (command* cmd, status* stat) {
	
	fflush(stdout);																			//Flush the stdout buffer before forking
	pid_t spawnPid = fork();																//Create a child and become a parent
	switch(spawnPid)
	{
		case 0:
		//This is the little baby spawnling child 
		
			execvp(cmd->argv[0], cmd->argv);												//Pass off the command and its arguments to be searched with the PATH env.
			
			//If this executes then we couldn't find the command under PATH env.
			printf("Error: Command:( %s ) was not found!\n", cmd->argv[0]);
			fflush(stdout);
			stat->status = 1;																//Set the return status to 1
			exit(1);
			
		case -1:
		//This is if we have an error forking
			printf("Error forking\n");
			fflush(stdout);
			exit(1);
			
		default:
		//This is the parent
			//Need to wait and listen for the child's exit status!
			waitpid(spawnPid, &stat->status, 0);											//Wait for the child's exit status and save it in the status struct.
		
	}
}

void handOffOut(command* cmd, status* stat) {
	//Need to redirect output of the command to a file that is given.
	int file_descriptor;
	
	fflush(stdout);																			//Flush the stdout buffer before forking
	pid_t spawnPid = fork();																//Create a child and become a parent
	switch(spawnPid)
	{
		case 0:
		//This is the little baby spawnling child 
			/*Check if the argument to > exists
			if(cmd->argv[cmd->oIdx+1] != NULL) 
			{
				stat.status = 1;
				exit(1);
			}
			*/
			
			file_descriptor = open(cmd->argv[cmd->oIdx + 1], O_RDWR | O_CREAT, 0770); 			//Open the given file name
			//remove the two arguments from the command so they arent taken as arguments to the ACTUAL COMMAND
			cmd->argv[cmd->oIdx + 1] = NULL;
			cmd->argv[cmd->oIdx] = NULL;
			
			//Redirect stdout (1) to the file!
			dup2(file_descriptor, 1);
			//Redirect stderror (2) to the file!
			dup2(file_descriptor, 2);
			
			//Actually execute the command
			execvp(cmd->argv[0], cmd->argv);												//Pass off the command and its arguments to be searched with the PATH env.
			
			
			//If this executes then we couldn't find the command under PATH env.
			printf("Error: Command:( %s ) was not found!\n", cmd->argv[0]);
			fflush(stdout);
			stat->status = 1;																//Set the return status to 1
			
			//Also close the file
			close(file_descriptor);
			
			exit(1);
			
		case -1:
		//This is if we have an error forking
			printf("Error forking\n");
			fflush(stdout);
			exit(1);
			
		default:
		//This is the parent
			//Need to wait and listen for the child's exit status!
			waitpid(spawnPid, &stat->status, 0);											//Wait for the child's exit status and save it in the status struct.
		
	}
}

void handOff (command* cmd, status* stat) {
	//Handle for normal, input, output, and background processes.
	if ( (cmd->backGround == 0) && (cmd->input == 0) && (cmd->output == 0) )
	{
		//Regular execution without any modifiers
		printf("Doing a normal command execution!\n");
		fflush(stdout);
		
		handOffExec(cmd, stat);
	}
	else if ( (cmd->backGround == 0) && (cmd->input == 0) && (cmd->output == 1) )
	{
		//Handle output redirection
		printf("Doing an output redirection command execution!\n");
		fflush(stdout);
		
		handOffOut(cmd, stat);
		
	}
	else if ( (cmd->backGround == 0) && (cmd->input == 1) && (cmd->output == 0) )
	{
		//Handle input redirection
		printf("Doing an input redirection command execution!\n");
		fflush(stdout);
		
	}
	else if ( (cmd->backGround == 1) && (cmd->input == 0) && (cmd->output == 0) )
	{
		//Handle background process
		printf("Doing a background command execution!\n");
		fflush(stdout);
	}
}









int main() {
	command cmd;																			//Create an instance of my command struct
	cmd.argc = 0;																			//Set the unitialized argument count to 0;
	//cmd.input = 0;
	//cmd.iIdx = 0;
	//cmd.output = 0;
	//cmd.oIdx = 0;
	//cmd.backGround = 0;
	
	int again = 0;
	
	status stat;																			//Keeps track of the exit status of a previous command/program (can be a terminiation signal?).
	stat.status = 0;																		//Starts at 0
	stat.signal = 0;																		//Set to zero, because there is no signal with 0 (Basically uninitialized).
	
	
	while (again == 0) 																		//Repeat prompting and handling commands
	{
		//Prompt user for input
		prompt(&cmd);
		//printCommand(cmd);
		
		//check if the command (first argument) is a newline and (first charcter of the first argument) is a comment!
		if( strcmp(cmd.argv[0], "\n") && (cmd.argv[0][0] != '#') )
		{
			//This is where we can check for our versions of cd, exit, and status
			if( !strcmp(cmd.argv[0], "exit") )												//If we enter exit, we just exit the shell (aka this program/process)
			{
				printf("EXITING smallsh Session!\n");
				fflush(stdout);
				again = 1;
				return 0;
			}
			else if( !strcmp(cmd.argv[0], "cd") )
			{
				//Perform my version of changing the directory
				
				//printf("Changing Directory\n");
				//fflush(stdout);
				myCD(&cmd);																	//Pass myCD the commmand.
			}
			else if( !strcmp(cmd.argv[0], "status") )
			{
				//Print the status of the previously executed command/program (can be a terminiation signal?)
				
				//printf("Showing status\n");
				//fflush(stdout);
				printmyStatus(stat);
			}
			else 
			{
				//Then handle other commands
				
				//printf("Passing off command to shell/OS?\n");
				//fflush(stdout);
				handOff(&cmd, &stat);														//Decide how to hand off the command to the OS/Shell
			}
		
		}
	}
	
	return 0;
}