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

//**Global Variable
sig_atomic_t SIGTSTPFlag = 0; 																				//Set a global bool/flag for keeping track of what mode we are in


//**STRUCTS:
//defines command as a standard type for use
typedef struct cmmd command;
//Structure of commands that will be used
struct cmmd {
		int argc;																							//Keeps count of the number of arguments
		char* argv[512];																					//Set max of arguments to (**MAX NEEDS TO BE 512 ARGS**) characters (includes the actual command along side arguments)
		int bg;																								//Keeps track if this is a background proccess? (bool)
		int input;																							//Wether or not we need to redirect input (bool)
		int iIdx;																							//Stores the index of the input operator
		int output;																							//Wether or not we need to redirect input (bool)
		int oIdx;																							//Stores the index of the output operator
};

//Destructor
/******************************************************************
 * * ** Function: freeCommand(command* cmd)
 * ** Description: Destructs and free's a command structure's heap memmory
 * ** Parameters: command* cmd
 * ** Pre-conditions: Given a heaped command
 * ** Post-conditions: Free's the command
 * *******************************************************************
 */
void freeCommand(command* cmd)
{
	//Go through all the arguments starting from the last argument.
	while(cmd->argc != 0)														
	{
		//printf("In loop\n");
		//fflush(stdout);
		
		if (!cmd->argv[cmd->argc])																			//If the argument exists, free it
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
	int status;																								//Keeps track of the exit status of a previous command/program 
	int signal;																								//Keeps track if we have recieved the CTRL-Z signal an is a bool/toggle
	int bgNum;																								//Keeps track of how many bg processes are currently running.
	pid_t bgPid[20];																						//Keeps hold of 20 background process ID's
};


/******************************************************************
 * * ** Function: printmyStatus(status stat)
 * ** Description: Prints the exit/termination status of a status structure
 * ** Parameters: status stat
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void printmyStatus(status stat) {
	//Check if there a termination signal was recieved with the last process
	/*
	if( stat.signal != 0 )
	{
		//If a signal was termination recieved from the last process, print out the singal number.
		printf("Terminated by signal: %d\n", WTERMSIG(stat.status) );
		fflush(stdout);
	}
	else 
	{
		//If no signal was recieved print out the exit status
		printf("Exit status of: %d\n", WEXITSTATUS(stat.status) );
		fflush(stdout);
	}
	*/
	if( WIFSIGNALED(stat.status) )
	{
		printf("Terminated by signal: %d\n", WTERMSIG(stat.status) );
		fflush(stdout);
	}
	else
	{
		printf("Exit status of: %d\n", WEXITSTATUS(stat.status));
		fflush(stdout);
	}
}


//**FUNCTIONS:
/******************************************************************
 * * ** Function: printStr(char* str)
 * ** Description: Prints the given string with fflush
 * ** Parameters: char* str
 * ** Pre-conditions: Given an actual NULL terminated string
 * ** Post-conditions: Prints it out to stdout and flushes the buffer
 * *******************************************************************
 */
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

/******************************************************************
 * * ** Function: waitBg(status* stat)
 * ** Description: Takes a status struct and checks if a background processe has finished execution and prints out the exit/termination status.
 * ** Parameters: status* stat
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void waitBg(status* stat) {
	//Check if we actually have any processes in the background.
	//printf("In waitBg!\n");
	//fflush(stdout);
	
	if(stat->bgNum != 0)
	{
		
		//Go through all of the background processes
		for (int i = 0; i < stat->bgNum; i++)
		{
			//Check if this process is done processing
			if( waitpid(stat->bgPid[i], &stat->status, WNOHANG) )
			{
				//This means that this child/process has finished and its time to dine/begin!
				printf("background pid %d is done: ", (int) stat->bgPid[i] );								//Print out that the background process has finished.
				fflush(stdout);
				
				//Get rid of the Pid of the finished process in the array
				//
				for(int j = i; j < (stat->bgNum - 1); j++)
				{
					//check if the next pid is NULL
					if(stat->bgPid[j + 1])
					{
						//Move the next pid down an index
						stat->bgPid[j] = stat->bgPid[j + 1];
					}
					else
					{
						//If the next PID is NULL break out of this loop as the job is done!
						break;
					}
				}
				//
				stat->bgNum -= 1;																			//Decrement the amount of processes running
				
				//Check if the exit status is something we can interpret
				if( WIFSIGNALED(stat->status) )
				{
					printf("terminated by signal %d\n", WTERMSIG(stat->status) );
					fflush(stdout);
				}
				else
				{
					printf("exit value %d\n", WEXITSTATUS(stat->status));
					fflush(stdout);
				}
				
			}
			
		}
		
	}
}

/******************************************************************
 * * ** Function: expandString(char* token)
 * ** Description: Parses through a string searching for $$ and expands it to the shell's Pid and allocates heap memeory for the expanded string.
 * ** Parameters: status* stat
 * ** Pre-conditions: 
 * ** Post-conditions: Returns the expanded string
 * *******************************************************************
 */
char* expandString(char* token) {
	//This is a mess
	int currPid = getpid();
	
	char* clone = token;
	
	char* ret;
	char pidBuff[20];
	//Convert the currPid to a string
	sprintf(pidBuff, "%lld", currPid);
	
	//int diff;
	size_t diff;
	
	int finish = 0;

	char result[2000];
	memset(result, '\0', sizeof(result));
	
	char* temp2;
	
	char temp1[2000];
	memset(temp1, '\0', sizeof(temp1));
	
	//Search if this string needs to be expanded. For the first time
	ret = strstr(clone, "$$");
	
	
	
	//Perhaps use a do while? With an if statement?
	do
	{
		
		//If $$ was found
		if(ret)
		{
			//calculate difference between the start of the string and where the $$ sign is if it exists.
			diff = ret - clone;
			
			//printf("The $$ is found at index: %d\n", diff);
			//fflush(stdout);
			
			//Copy that difference into a temp string if its not 0
			if(diff != 0)
			{
				//Clear temp1
				memset(temp1, '\0', sizeof(temp1));
				//Copy what we want
				strncpy(temp1, clone, diff);
				//Cat that to results
				strcat(result, temp1);
			}

			
			//Cat it into the results
			strcat(result, pidBuff);
			
			//Now add the rest of the string to the result.
			ret += 2;																						//Move the start of return just past the $$
			
			//Check if there is anything past the $$
			if(ret != NULL)
			{
				//strcat(result, ret);
			}
			//If there is nothing we need to break out of the loop
			else
			{
				break;
			}
			
			//printf("result is now: %s\n", result);
			//fflush(stdout);
			
			//Update the token to point to the next part of the token to check for $$
			//token = (ret - 2);
			//token = result;
			clone = ret;
			finish = 1;
			
			//printf("token is now: %s\n", clone);
			//fflush(stdout);
		}
		
	} while( (ret = strstr(clone, "$$")) != NULL );
	
	//Allocate memory for this argument and return it
	if(finish == 1)
	{
		temp2 = malloc(sizeof(strlen(result)) );
		memset( temp2, '\0', sizeof(temp2) );																//Clears the allocated string memory for copying.
		strcpy(temp2, result);
	}
	else
	{
		temp2 = malloc(sizeof(strlen(token)) );
		memset( temp2, '\0', sizeof(temp2) );																//Clears the allocated string memory for copying.
		strcpy(temp2, token);
	}
	
	//printf("Clone is: %s\n", clone);
	//fflush(stdout);
	
	//printf("Token is: %s\n", token);
	//fflush(stdout);
	
	//printf("Formatted argument string is now: %s\n", temp2);
	//fflush(stdout);
	
	return temp2;
}

/******************************************************************
 * * ** Function: prompt(command* cmd)
 * ** Description: Prompts the user for input and parses through the input and sets necessary flags and insterst data into the command struct.
 * ** Parameters: command* cmd
 * ** Pre-conditions: 
 * ** Post-conditions:
 * *******************************************************************
 */
void prompt(command* cmd) {
	//NEED TO RESET THE COMMAND CMD SO THAT IT WONT JUST CONTAIN THE PREVIOUS COMMAND'S ARGUMENTS/VALUES AND STUFF AND TO NOT LEAK MEMORY!!!!!!!!
	freeCommand(cmd);
	
	char* token;
	char* saveptr;																							//Using this variable for strtok_r
	
	char str_input[2049];																					//Set maximum amount of characters that we can take in to 2048 (excluding null character?)
	memset(str_input, '\0', sizeof(str_input));																//Clears the string memory to only NULL
	printf(": ");																							//Character to denote that we are ready for a new command.
	fflush(stdout);
	
	//Read in a line from the user
	fgets(str_input, 2048, stdin);																			//take 2048 just to make sure we have enough room in the string for a NULL terminator
	//printf("The command you input is: %s\n", str_input);
	//fflush(stdout);
	
	//check if we have filled the string and that it is greater than 1
	if( (str_input[strlen(str_input)-1] == '\n') && (strlen(str_input) > 1) )
	{
		str_input[strlen(str_input)-1] = '\0';																//Replace the newline with a null terminator so that we can tokenize this string and parse it.
	}
	//printf("The command you input w/out a (natural) '\\n' is: %s\n", str_input);
	//fflush(stdout);
	
	
	//Parse the input
	
	token = strtok_r(str_input, " ", &saveptr); 															//Grabs the first argument (the actual command) 
	//printStr(token);
	
	
	while(token != NULL) 
	{
		//Check this argument for $$ and expand
		cmd->argv[cmd->argc] = expandString(token);
		
		
		/*Grab and store the data in the cmd struct
		cmd->argv[cmd->argc] = malloc( sizeof(strlen(token)) ); 											//allocate memory for the new command
		memset( cmd->argv[cmd->argc], '\0', sizeof(cmd->argv[cmd->argc]) );									//Clears the allocated string memory for copying.
		strcpy(cmd->argv[cmd->argc], token);
		*/
		
		cmd->argc += 1;																						//increment the number of arguments
		//
		
		token = strtok_r(NULL, " ", &saveptr); 																//Grab the next argument (if it doesnt exist it will be NULL)
		
		/*Prints each token that is aquired.
		if(token != NULL) 
		{
			printStr(token);
		}
		*/
	}
	

	//Set the last cmd argument to NULL for if/when we call the process in an exec
	cmd->argv[cmd->argc] = NULL;
	
	//printf("Checking if there is a background process!\n");
	//fflush(stdout);
	
	//Check if this needs to be set as a background process? ( '&' the background notifier will always be the last argument of each command besides NULL.)
	if( cmd->argc != 0)
	{
		if( !strcmp(cmd->argv[cmd->argc - 1], "&") )														//Check if the & exists at the end of the command!
		{
			//Set the background process bool to true.
			cmd->bg = 1;
			
			//Make sure we get rid of the & so it isnt used as an argument when we execute the command.
			free(cmd->argv[cmd->argc - 1]);
			//Now change it to NULL so we know where the arguments end.
			cmd->argv[cmd->argc - 1] = NULL;
			
			//Make sure to lower the argument count by one because & isn't an argument.
			cmd->argc -= 1;																
		}
		//If the & hasn't been found
		else
		{
			//Set the background process bool to false.
			cmd->bg = 0;
		}
	}
	
	//Check if we need to do any redirection
	//Reset redirection variables
	cmd->output = 0;
	cmd->oIdx == 0;
	cmd->input = 0;
	cmd->iIdx = 0;
	
	//Go through all of the arguments, just need to identify if the arguments exist and where this argument is
	for(int i = 0; i < cmd->argc; i++)
	{
		//If the current argument is NULL, exit
		if(cmd->argv[i] == NULL)
		{
			break;
		}
		
		//Actually check if the current argument is an alligator ( < or > )
		if( strcmp(cmd->argv[i], "<") == 0 )
		{
			//Means we are redirecting input
			cmd->input = 1;																					//Set the input bool to true
			cmd->iIdx = i;																					//Store the current index of the redirect operator
		}
		if( strcmp(cmd->argv[i], ">") == 0 )
		{
			//Means we are redirecting output
			cmd->output = 1;																				//Set the input bool to true
			cmd->oIdx = i;																					//Store the current index of the redirect operator
		}
	}
	
}

/******************************************************************
 * * ** Function: myCD(command* cmd)
 * ** Description: Moves to a specified directory based on user input.
 * ** Parameters: command* cmd
 * ** Pre-conditions: 
 * ** Post-conditions: Working directory is moved if found.
 * *******************************************************************
 */
void myCD(command* cmd) {
	//Check if we actually have a second argument to cd
	if(cmd->argv[1] != NULL)
	{
		//Check if the directory exists!
		DIR* targetDir = opendir(cmd->argv[1]);																//Attempt to open the targeted directory/path
		if(targetDir != NULL) 
		{
			//The directory exists and change to the targeted directory
			//printf("Changing to Directory: %s\n", cmd->argv[1] );
			//fflush(stdout);
			
			chdir(cmd->argv[1]);
			closedir(targetDir);																			//Close the directory since we know it exists now.
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
		
		DIR* targetDir = opendir( getenv("HOME") );															//Grabs the path of the HOME env and opens the directory.
		chdir( getenv("HOME") );																			//Change the current working directory to that HOME path.
		closedir(targetDir);																				//Don't need to keep the directory open...
		return;
	}
}

/******************************************************************
 * * ** Function: handOffExec (command* cmd, status* stat)
 * ** Description: Hand's off a cmd to be executed and changes the I/O of a child based on the command's background flag.
 * ** Parameters: command* cmd, status* stat
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void handOffExec (command* cmd, status* stat) {
	int file_descriptor;
	
	struct sigaction SIGINT_default = {0};
	SIGINT_default.sa_handler = SIG_DFL;
	
	struct sigaction SIGTSTP_ignore = {0};
	SIGTSTP_ignore.sa_handler = SIG_IGN;
	
	fflush(stdout);																							//Flush the stdout buffer before forking
	pid_t spawnPid = fork();																				//Create a child and become a parent
	switch(spawnPid)
	{
		case 0:
		//This is the little baby spawnling child 
		
			//Ignore SIGTSTP no matter if we are a bg or fg
			sigaction(SIGTSTP, &SIGTSTP_ignore, NULL);
		
			//Check if this is a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Open up a tear in space time and send the data into the abyss.
				file_descriptor = open("/dev/null",O_WRONLY);
			
				//Redirect stdout (1) to the void
				dup2(file_descriptor, 1);
				//Redirect sterr (2) to the void
				dup2(file_descriptor, 2);
			}
			else
			{
				//If not a background process make it so we can CTRL-C this foreground process
				sigaction(SIGINT, &SIGINT_default, NULL);
			}
		
		
			execvp(cmd->argv[0], cmd->argv);																//Pass off the command and its arguments to be searched with the PATH env.
			
			//If this executes then we couldn't find the command under PATH env.
			printf("Error: Command:( %s ) was not found!\n", cmd->argv[0]);
			fflush(stdout);
			stat->status = 1;																				//Set the return status to 1
			exit(1);			
			
		case -1:
		//This is if we have an error forking
			printf("Error forking\n");
			fflush(stdout);
			exit(1);
			
		default:
		//This is the parent
			//Check if this a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Save the process id of this parent so we can reap the corpse in the actual shell.
				stat->bgNum += 1;
				stat->bgPid[stat->bgNum - 1] = spawnPid;  
				
				printf("background pid created: %d\n", (int) spawnPid );									//Print out that the background process has finished.
				fflush(stdout);
			}	
			else
			{
				//Need to wait and listen for the child's exit status!
				waitpid(spawnPid, &stat->status, 0);														//Wait for the child's exit status and save it in the status struct.
			}																								
		
	}
}

/******************************************************************
 * * ** Function: handOffOut (command* cmd, status* stat)
 * ** Description: Hand's off a cmd to be executed and changes the I/O of a child based on the command's flags and changes output to a specified file.
 * ** Parameters: command* cmd, status* stat
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void handOffOut(command* cmd, status* stat) {
	//Need to redirect output of the command to a file that is given.
	int file_descriptor;
	int file_descriptor2;
	
	struct sigaction SIGINT_default = {0};
	SIGINT_default.sa_handler = SIG_DFL;
	
	struct sigaction SIGTSTP_ignore = {0};
	SIGTSTP_ignore.sa_handler = SIG_IGN;
	
	fflush(stdout);																							//Flush the stdout buffer before forking
	pid_t spawnPid = fork();																				//Create a child and become a parent
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
			
			//Ignore SIGTSTP no matter if we are a bg or fg
			sigaction(SIGTSTP, &SIGTSTP_ignore, NULL);
			
			//Check if this is a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Open up a tear in space time and send the data into the abyss.
				file_descriptor2 = open("/dev/null",O_WRONLY);
			
				//Redirect stdout (1) to the void
				dup2(file_descriptor2, 1);
				//Redirect sterr (2) to the void
				dup2(file_descriptor2, 2);
			}
			else
			{
				//If not a background process make it so we can CTRL-C this foreground process
				sigaction(SIGINT, &SIGINT_default, NULL);
			}
			
			file_descriptor = open(cmd->argv[cmd->oIdx + 1], O_WRONLY | O_CREAT | O_TRUNC, 0660); 			//Open the given file name
			//remove the two arguments from the command so they arent taken as arguments to the ACTUAL COMMAND
			cmd->argv[cmd->oIdx + 1] = NULL;
			cmd->argv[cmd->oIdx] = NULL;
			
			//Redirect stdout (1) to the file!
			dup2(file_descriptor, 1);
			//Redirect stderror (2) to the file!
			dup2(file_descriptor, 2);
			
			
			//Actually execute the command
			execvp(cmd->argv[0], cmd->argv);																//Pass off the command and its arguments to be searched with the PATH env.
			
			
			//If this executes then we couldn't find the command under PATH env.
			printf("Error: Executing command!\n");
			fflush(stdout);
			stat->status = 1;																				//Set the return status to 1
			
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
			//Check if this a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Save the process id of this parent so we can reap the corpse in the actual shell.
				stat->bgNum += 1;
				stat->bgPid[stat->bgNum - 1] = spawnPid;  
				
				printf("background pid created: %d\n", (int) spawnPid );									//Print out that the background process has finished.
				fflush(stdout);
			}	
			else
			{
				//Need to wait and listen for the child's exit status!
				waitpid(spawnPid, &stat->status, 0);														//Wait for the child's exit status and save it in the status struct.
			}
	}
}

/******************************************************************
 * * ** Function: handOffIn (command* cmd, status* stat)
 * ** Description: Hand's off a cmd to be executed and changes the I/O of a child based on the command's flags and changes input to a specified file.
 * ** Parameters: command* cmd, status* stat
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void handOffIn(command* cmd, status* stat) {
	//Need to redirect input of the command from a file that is given.
	
	struct sigaction SIGINT_default = {0};
	SIGINT_default.sa_handler = SIG_DFL;
	
	struct sigaction SIGTSTP_ignore = {0};
	SIGTSTP_ignore.sa_handler = SIG_IGN;
	
	int file_descriptor = open(cmd->argv[cmd->iIdx + 1], O_RDONLY); 										//Open the given file name
	int file_descriptor2;
	//Check if it opened properly
	if(file_descriptor == -1)
	{
		printf("Error: Was not able to open file: %s\n", cmd->argv[cmd->iIdx + 1]);
		fflush(stdout);
		stat->status = 1;
		//exit(1);
	}
	
	//Now fork and execute the command
	fflush(stdout);																							//Flush the stdout buffer before forking
	pid_t spawnPid = fork();																				//Create a child and become a parent
	switch(spawnPid)
	{
		case 0:
		//This is the little baby spawnling child 
			
			//Ignore SIGTSTP no matter if we are a bg or fg
			sigaction(SIGTSTP, &SIGTSTP_ignore, NULL);
			
			//Check if this is a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Open up a tear in space time and send the data into the abyss.
				file_descriptor2 = open("/dev/null",O_WRONLY);
			
				//Redirect stdout (1) to the void
				dup2(file_descriptor2, 1);
				//Redirect sterr (2) to the void
				dup2(file_descriptor2, 2);
			}
			else
			{
				//If not a background process make it so we can CTRL-C this foreground process
				sigaction(SIGINT, &SIGINT_default, NULL);
			}
			
			//remove the two arguments from the command so they arent taken as arguments to the ACTUAL COMMAND
			cmd->argv[cmd->iIdx + 1] = NULL;
			cmd->argv[cmd->iIdx] = NULL;
			
			//Redirect stdin (0) to the file!
			dup2(file_descriptor, 0);
			
			
			//Actually execute the command
			execvp(cmd->argv[0], cmd->argv);																//Pass off the command and its arguments to be searched with the PATH env.
			
			//If this executes then we couldn't find the command under PATH env.
			printf("Error: Executing command!\n");
			fflush(stdout);
			stat->status = 1;																				//Set the return status to 1
			
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
			//Check if this a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Save the process id of this parent so we can reap the corpse in the actual shell.
				stat->bgNum += 1;
				stat->bgPid[stat->bgNum - 1] = spawnPid;  
				
				printf("background pid created: %d\n", (int) spawnPid );									//Print out that the background process has finished.
				fflush(stdout);
			}	
			else
			{
				//Need to wait and listen for the child's exit status!
				waitpid(spawnPid, &stat->status, 0);														//Wait for the child's exit status and save it in the status struct.
			}
		
	}
}

/******************************************************************
 * * ** Function: handOffBoth (command* cmd, status* stat)
 * ** Description: Hand's off a cmd to be executed and changes the I/O of a child based on the command's flags and changes input and output to a specified file.
 * ** Parameters: command* cmd, status* stat
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void handOffBoth(command* cmd, status* stat) {
	//Need to redirect input of the command from a file that is given.
	
	struct sigaction SIGINT_default = {0};
	SIGINT_default.sa_handler = SIG_DFL;
	
	struct sigaction SIGTSTP_ignore = {0};
	SIGTSTP_ignore.sa_handler = SIG_IGN;
	
	int file_descriptor1 = open(cmd->argv[cmd->iIdx + 1], O_RDONLY); 										//Open the given file name
	//Check if it opened properly
	if(file_descriptor1 == -1)
	{
		printf("Error: Was not able to open file: %s\n", cmd->argv[cmd->iIdx + 1]);
		fflush(stdout);
		stat->status = 1;
		//exit(1);
	}
	
	int file_descriptor2;
	int file_descriptor3;
	
	//Now fork and execute the command
	fflush(stdout);																							//Flush the stdout buffer before forking
	pid_t spawnPid = fork();																				//Create a child and become a parent
	switch(spawnPid)
	{
		case 0:
		//This is the little baby spawnling child 
			
			//Ignore SIGTSTP no matter if we are a bg or fg
			sigaction(SIGTSTP, &SIGTSTP_ignore, NULL);
			
			//Check if this is a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Open up a tear in space time and send the data into the abyss.
				file_descriptor3 = open("/dev/null",O_WRONLY);
			
				//Redirect stdout (1) to the void
				dup2(file_descriptor3, 1);
				//Redirect sterr (2) to the void
				dup2(file_descriptor3, 2);
			}
			else
			{
				//If not a background process make it so we can CTRL-C this foreground process
				sigaction(SIGINT, &SIGINT_default, NULL);
			}
		
			
			
			//remove the two arguments from the command so they arent taken as arguments to the ACTUAL COMMAND
			cmd->argv[cmd->iIdx + 1] = NULL;
			cmd->argv[cmd->iIdx] = NULL;
			
			//Redirect stdin (0) to the file!
			dup2(file_descriptor1, 0);
			
			file_descriptor2 = open(cmd->argv[cmd->oIdx + 1], O_WRONLY | O_CREAT | O_TRUNC, 0660); 			//Open the given file name
			//remove the two arguments from the command so they arent taken as arguments to the ACTUAL COMMAND
			cmd->argv[cmd->oIdx + 1] = NULL;
			cmd->argv[cmd->oIdx] = NULL;
			
			//Redirect stdout (1) to the file!
			dup2(file_descriptor2, 1);
			//Redirect stderror (2) to the file!
			dup2(file_descriptor2, 2);
			
			
			//Actually execute the command
			execvp(cmd->argv[0], cmd->argv);																//Pass off the command and its arguments to be searched with the PATH env.
			
			//If this executes then we couldn't find the command under PATH env.
			printf("Error: Executing command!\n");
			fflush(stdout);
			stat->status = 1;																				//Set the return status to 1
			
			//Also close the file
			close(file_descriptor1);
			
			exit(1);
			
		case -1:
		//This is if we have an error forking
			printf("Error forking\n");
			fflush(stdout);
			exit(1);
			
		default:
		//This is the parent
			//Check if this a background process
			if( (cmd->bg == 1) && (SIGTSTPFlag == 0) )
			{
				//Save the process id of this parent so we can reap the corpse in the actual shell.
				stat->bgNum += 1;
				stat->bgPid[stat->bgNum - 1] = spawnPid;  
				
				printf("background pid created: %d\n", (int) spawnPid );									//Print out that the background process has finished.
				fflush(stdout);
			}	
			else
			{
				//Need to wait and listen for the child's exit status!
				waitpid(spawnPid, &stat->status, 0);														//Wait for the child's exit status and save it in the status struct.
			}

	}
}


/******************************************************************
 * * ** Function: handOff (command* cmd, status* stat)
 * ** Description: Determines how to handle a given command and what the command needs to do. I/O, etc.
 * ** Parameters: command* cmd, status* stat
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void handOff (command* cmd, status* stat) {
	//Handle for normal, input, output, and background processes.
	if ( (cmd->input == 0) && (cmd->output == 0) )
	{
		//Regular execution without any modifiers
		//printf("Doing a normal command execution!\n");
		//fflush(stdout);
		
		handOffExec(cmd, stat);
	}
	else if ( (cmd->input == 0) && (cmd->output == 1) )
	{
		//Handle output redirection
		//printf("Doing an output redirection command execution!\n");
		//fflush(stdout);
		
		handOffOut(cmd, stat);
		
	}
	else if ( (cmd->input == 1) && (cmd->output == 0) )
	{
		//Handle input redirection
		//printf("Doing an input redirection command execution!\n");
		//fflush(stdout);
		
		handOffIn(cmd, stat);
		
	}
	else if ( (cmd->input == 1) && (cmd->output == 1) )
	{
		//Handle both redirecting input and output.
		//printf("Doing a double redirection command execution!\n");
		//fflush(stdout);
		
		handOffBoth(cmd, stat);
	}
}

/******************************************************************
 * * ** Function: handle_SIGTSTP(int signo)
 * ** Description: Signal handler for sigTSTP which 'toggles' a global flag for changing background modes.
 * ** Parameters: int signo
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
void handle_SIGTSTP(int signo) {
	
	//char* message = "Entering foreground-only mode (& is now ignored)\n";
    //write(STDOUT_FILENO, message, 49);
	
	
    if (SIGTSTPFlag == 0)
	{
		//If we are anot in foreground mode
        char* msg1 = "Entering foreground-only mode (& is now ignored)\n";
        SIGTSTPFlag = 1;
        write(STDOUT_FILENO, msg1, 49);
    }
    else
	{
		//If we are in foreground mode
        char* msg2 = "Exiting foreground-only mode\n";
        SIGTSTPFlag = 0;
        write(STDOUT_FILENO, msg2, 29);
    }
	
}

/******************************************************************
 * * ** Function: main()
 * ** Description: This is where the magic happens and the actual shell and structure setup is done. Decides the first layer of what kind of a command was recieved and how to handle it.
 * ** Parameters: 
 * ** Pre-conditions: 
 * ** Post-conditions: 
 * *******************************************************************
 */
int main() {
	//Signaaction structure setup
	struct sigaction SIGINT_default = {0}, SIGINT_ignore = {0}, SIGTSTP_action = {0};						//Declare three new sigaction structs for use

	SIGINT_default.sa_handler = SIG_DFL;																	//Set this handler to the default
	SIGINT_ignore.sa_handler = SIG_IGN;																		//Set this handler to ignore
	
	//Ignore CTRL-C
	sigaction(SIGINT, &SIGINT_ignore, NULL);																//Ignore CTRL-C signals
	
	SIGTSTP_action.sa_handler = handle_SIGTSTP;																//Point the handler to my handler function
	sigfillset(&SIGTSTP_action.sa_mask);																	//Ignore all flags when the handler is run
	SIGTSTP_action.sa_flags = 0;
	
	//Catch CTRL-Z
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);																//Catch CTRL-Z signals with our signal handler.
	
	
	command cmd;																							//Create an instance of my command struct
	cmd.argc = 0;																							//Set the unitialized argument count to 0;
	//cmd.input = 0;
	//cmd.iIdx = 0;
	//cmd.output = 0;
	//cmd.oIdx = 0;
	//cmd.backGround = 0;
	
	int again = 0;
	
	status stat;																							//Keeps track of the exit status of a previous command/program (can be a terminiation signal?).
	stat.status = 0;																						//Starts at 0
	stat.signal = 0;																						//Set to zero, because there is no signal with 0 (Basically uninitialized).
	stat.bgNum = 0;																							//Set to zero, because there are no background processes currently
	memset(stat.bgPid, '\0', sizeof(stat.bgPid));															//Set the array of background process id's to NULL becaus there arent any.
	
	
	while (again == 0) 																						//Repeat prompting and handling commands
	{
		//Before prompting for user input we need to check for finished background processes.
		waitBg(&stat);
		
		//Prompt user for input
		prompt(&cmd);
		//printCommand(cmd);
		
		//check if the command (first argument) is a newline and (first charcter of the first argument) is a comment!
		if( cmd.argc != 0)
		{
			if( strcmp(cmd.argv[0], "\n") && (cmd.argv[0][0] != '#') )
			{
				//This is where we can check for our versions of cd, exit, and status
				if( !strcmp(cmd.argv[0], "exit") )															//If we enter exit, we just exit the shell (aka this program/process)
				{
					//printf("EXITING smallsh Session!\n");
					//fflush(stdout);
					again = 1;
					return 0;
				}
				else if( !strcmp(cmd.argv[0], "cd") )
				{
					//Perform my version of changing the directory
					
					//printf("Changing Directory\n");
					//fflush(stdout);
					myCD(&cmd);																				//Pass myCD the commmand.
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
					handOff(&cmd, &stat);																	//Decide how to hand off the command to the OS/Shell
				}
			
			}
		
		}
	}
	
	return 0;
}