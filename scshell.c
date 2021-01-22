
#include <stdio.h>
#include <unistd.h> // fork
#include <string.h>
#include <stdlib.h> // exit
#include <sys/types.h> // pid_t
#include <sys/wait.h> // wait
#include <fcntl.h> // open

// For clear screen
#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#define clrscr() printf("\e[1;1H\e[2J")
#endif

#define MAX_LENGTH 80 // The maximum length of the commands

void init_shell() //Open when launch shell
{
	clrscr();
	printf("\n================================================");
	printf("\n|              SIMPLE C SHELL                  |");
	printf("\n|______________________________________________|");
	printf("\n|  This simple shell on C                      |");
	printf("\n| C lang by:  Dennis Ritchie                   |");
	printf("\n|               Shell by: lik173               |");
	printf("\n================================================");
	char *username = getenv("USER");
	printf("\n|   Type 'list'or 'help' for get help.         |");
	printf("\n================================================");
	printf("\n\n");
}

void help_panel()
{
	puts("\n____________________________________________"
		 "\n|___________________HELP___________________|"
		 "\n|__________________________________________|"
		 "\n| Help list of scshell                     |"
		 "\n|     history                              |"
		 "\n|     ls (Shows files in directory)        |"
		 "\n|     pwd (Directory where are you)        |"
		 "\n|     ping (Net Tool)                      |"
		 "\n|     exit                                 |"
		 "\n|     info (Get info)                      |"
		 "\n|     beer (99 bottles of beer)            |"
		 "\n|__________________________________________|\n");

	return;
}


void info_panel()
{
	puts("\n____________________________________________"
		 "\n|___________________INFO___________________|"
		 "\n|__________________________________________|"
		 "\n| Information about scshell:               |"
		 "\n|       C lang:    Dennis Ritchie          |"
		 "\n|       SCShell:    lik173                 |"
		 "\n|       Site(RU):    lik173.github.io      |"
		 "\n|       Version:    1.0 , patch 0          |"
		 "\n|__________________________________________|"
		 "\n| Have idea, update, code, question?       |"
		 "\n|   Meet me on Github!                     |"
		 "\n|      <github.com/lik173/scshell/issue>   |"
		 "\n|__________________________________________|\n");

	return;
}

void nohist_panel()
{
	puts("\n____________________________________________"
		 "\n|__________________HISTORY_________________|"
		 "\n|__________________________________________|"
		 "\n|                                          |"
		 "\n|           No symbols in history          |"
		 "\n|                                          |"
		 "\n|__________________________________________|\n");

	return;
}


void beer_text()

{
  int n;
 
  for( n = 99; n > 2; n-- )
    printf(
      "%d bottles of beer on the wall, %d bottles of beer.\n"
      "Take one down and pass it around, %d bottles of beer on the wall.\n\n", 
       n, n, n - 1);
 
  printf(  
      "2 bottles of beer on the wall, 2 bottles of beer.\n"
      "Take one down and pass it around, 1 bottle of beer on the wall.\n\n"                  
 
      "1 bottle of beer on the wall, 1 bottle of beer.\n"
      "Take one down and pass it around, no more bottles of beer on the wall.\n\n"                  
 
      "No more bottles of beer on the wall, no more bottles of beer.\n" 
      "Go to the store and buy some more, 99 bottles of beer on the wall.\n");
 
}









void parse(char *command, char **args)
{
	while (*command != '\0') //if not the end of line, continue
	{
		while (*command == ' ' || *command == '\t' || *command == '\n')
		{
			*command++ = '\0'; //replace white spaces with 0
		}
		*args++ = command;
		while (*command != '\0' && *command != ' ' && *command != '\t' && *command != '\n')
		{
			command++;
		}
		*args = '\0'; //mark the end of argument list
	}
}

int findAmpersand(char **args)
{
	//Find "&"
	int count = 0;
	while (args[count] != NULL && strcmp(args[count], "&") != 0)
		count++;

	if (args[count] != NULL)
	{
		//Found
		args[count] = NULL; // Remove "&" to ensure that fork() will run properly.
		return 1;
	};
	return 0;
}

int findPipe(char **args)
{
	//Find "|""
	int count = 0;
	while (args[count] != NULL && strcmp(args[count], "|") != 0)
		count++;

	if (args[count] != NULL)
		//Found
		return 1;
	return 0;
}

int findIORedirect(char **args)
{
	// Find ">" or "<" symbol
	// Define: Return 0 - Not found
	//         Return 1 - Found ">" OUTPUT
	//         Return 2 - Found "<" INPUT
	int count = 0;
	while (args[count] != NULL && strcmp(args[count], ">") != 0 && strcmp(args[count], "<") != 0)
		count++;

	if (args[count] != NULL) {
		if (strcmp(args[count], ">") == 0)
			//Found ">"
			return 1;
		else if (strcmp(args[count], "<") == 0)
			//Found "<"
			return 2;
		else
			// Error handle
			return 0;
	};
	return 0;
}

void exec_w_Pipe(char **args)
{
	if (findPipe(args) != 1)
	{
		printf("\n*** PIPE: error, doing nothing.");
		return;
	}
	//Pre-process: Take argument before "|" and after.
	char *parsed[MAX_LENGTH];
	char *parsedpipe[MAX_LENGTH];

	int countTemp = 0, foundPipe = 0, argLen = 0, pipeLen = 0;
	while (args[countTemp] != NULL)
	{
		if (strcmp(args[countTemp], "|") == 0)
			foundPipe = 1;
		if (foundPipe == 0)
		{
			parsed[argLen] = args[countTemp];
			argLen++;
		}
		else if (strcmp(args[countTemp], "|") != 0)
		{
			parsedpipe[pipeLen] = args[countTemp];
			pipeLen++;
		};
		countTemp++;
	};

	// Mark the end of char array by "end" character
	parsed[argLen] = '\0';
	parsedpipe[pipeLen] = '\0';

	// Define:
	// fd[0] is read end, fd[1] is write end
	int fd[2];
	pid_t pipe1, pipe2;

	if (pipe(fd) < 0)
	{
		printf("\n*** PIPE: Init error.");
		return;
	}
	pipe1 = fork();
	if (pipe1 < 0)
	{
		printf("\n*** PIPE: Fork error.");
		return;
	}

	if (pipe1 == 0)
	{
		// pipe1
		// write at fd[1]
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO); //duplicate
		close(fd[1]);

		if (execvp(parsed[0], parsed) < 0)
		{
			printf("\n*** PIPE: Error when executing command befor pipe.");
			exit(0);
		}
	}
	else
	{
		// Parent execute
		pipe2 = fork();

		if (pipe2 < 0)
		{
			printf("\n*** PIPE: Fork parent error");
			return;
		}

		// Child 2 execute.
		// read at fd[0]
		if (pipe2 == 0)
		{
			close(fd[1]);
			dup2(fd[0], STDIN_FILENO); //duplicate
			close(fd[0]);
			if (execvp(parsedpipe[0], parsedpipe) < 0)
			{
				printf("\n*** PIPE: Error when executing command after pipe.");
				exit(0);
			}
		}
		else
		{
			// parent executing, waiting for two children
			wait(NULL);
		}
	}
}

void execute_w_io_redirect(int rType, char **args)
{
	// rType = 1: Output redirect
	// rType = 2: Input redirect
	
	// Double check if rType is valid
	if (rType <= 0)
		return;

	//Pre-process: Find argument after ">" or "<" symbol (file name)
	char* fileName;
	int countTemp = 0;
	while (args[countTemp] != NULL && strcmp(args[countTemp], ">") != 0 && strcmp(args[countTemp], "<") != 0)
		countTemp++;

	if (args[countTemp] != NULL && args[countTemp + 1] != NULL)
		if (strcmp(args[countTemp], ">") == 0 || strcmp(args[countTemp], "<") == 0)
			fileName = args[countTemp + 1];
		else
			return;
	else 
		// Error
		return;
	if (fileName == NULL) // Cannot find filename
		return;

	// Remove ">", "<" and file name to fork child process
	args[countTemp] = NULL;
	args[countTemp + 1] = NULL;

	pid_t pid;
	int status;
	if ((pid = fork()) < 0) // fork child process
	{
		printf("*** IOREDIRECT: forking child process failed.\n");
		exit(1);
	}
	else if (pid == 0)
	{
		// Child process
		switch (rType)
        {
            case 1:
            {
                // Output redirect
                int destination = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                dup2(destination, 1);
                break;
            }
			case 2:
            {
                // Input redirect
                int destination = open(fileName, O_RDONLY);
                if (destination < 0)
                {
                    printf("*** IOREDIRECT: File not found.\n");
                    exit(1);
                }
                dup2(destination, 0);
                break;
            }
            case 3:
            {
                // Error handle
                printf("*** IOREDIRECT: Unexpected error.\n");
                exit(1);
                break;
            }
        }

		if (execvp(*args, args) < 0)
		{
			printf("*** IOREDIRECT: Exec failed.\n");
			exit(1);
		}
	}
	else
	{
		while (pid != wait(&status))
		{
			printf("...\n");
		};
	}
}

void execute(char **args)
{
	int isPipe = findPipe(args);
	int isConcurrence = findAmpersand(args);
	int isIORedirect = findIORedirect(args);

	if (isPipe == 1)
	{
		//Execute if find pipe
		exec_w_Pipe(args);
	}
	else if (isIORedirect > 0) {
		execute_w_io_redirect(isIORedirect, args);
	}
	else
	{
		pid_t pid;
		int status;
		//Single command
		if ((pid = fork()) < 0) // fork child process
		{
			printf("*** HEY! forking child process failed.\n");
			exit(1);
		}
		else if (pid == 0)
		{
			if (execvp(*args, args) < 0)
			{
				printf("*** HEY! No such command or file\n");
				exit(1);
			}
		}
		else
		{
			// Execute at parent process
			if (isConcurrence == 1)
			{
				// Found "&"
				//Execute concurrence, immediately go back to parent
				return;
			};

			// Not found "&", waiting for child
			while (pid != wait(&status))
			{
				printf("...\n");
			};
		}
	}
}

int main(void)
{

	char command[MAX_LENGTH];
	char prevCommand[MAX_LENGTH];
	char *args[MAX_LENGTH / 2 + 1]; // MAximum 40 argments
	prevCommand[0] = '\0';
	int should_run = 1;
	init_shell();
	while (should_run)
	{
		printf("scsh> ");
		fflush(stdout);
		fgets(command, MAX_LENGTH, stdin);
		command[strlen(command) - 1] = '\0';
		if (strcmp(command, "") == 0)
			continue;
		else if (strcmp(command, "history") == 0)
		{
			if (strcmp(prevCommand, "") == 0)
			{
				nohist_panel();
				continue;
			}
			strncpy(command, prevCommand, strlen(prevCommand) + 1);
		}
		//Parse command and arguments.
		parse(command, args);
		if (strcmp(args[0], "exit") == 0)
			exit(0);
		else if (strcmp(args[0], "list") == 0)
			help_panel();
		else if (strcmp(args[0], "help") == 0)
			help_panel();
		else if (strcmp(args[0], "clear") == 0)
			clrscr();
		else if (strcmp(args[0], "cls") == 0)
			clrscr();
		else if (strcmp(args[0], "test") == 0)
			printf("Hello, World!\n");
		else if (strcmp(args[0], "info") == 0)
			info_panel();
		else if (strcmp(args[0], "beer") == 0)
			beer_text();
		else
		{
			strncpy(prevCommand, command, strlen(command) + 1);
			execute(args);
		}
	}
	return 0;
}
