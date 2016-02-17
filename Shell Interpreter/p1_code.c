#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

/* End the Background Processes */
void terminate_process(pid_t tpid[], int index)
{
	int i;
	for (i = 0; i < index; i++)
	{
		//Use SIGTERM to kill all the children	
		kill(tpid[i], SIGTERM);
	}
}

/*	char* getPrompt()
	Retrieve current working directory (see getcwd())
	if successful, returns complete prompt with absoluate path = "RSI: [cwd] >"
*/
char* getPrompt(){
	char cwd[1024];
	char answer[1024];
	char *returning;
	strcpy(answer, "RSI: ");

	/* use getcwd() to get current working directory */
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		strcat(cwd, " > ");
		strcat(answer, cwd);
		returning = answer;
		return returning;
	}else{
		perror("getcwd() error");
	}
	return NULL;
}

/*	main
*/
int main() {
	char* prompt;
	char* token;
	char* command;
	char* arg[100];      /* tokenized user input */
	int bailout = 1;
	int i, retval, background, index = 0;
	int rv = 0;
	int child_status;     /* child process: user-provided return code */
	pid_t childpid, tpid[40];
	
	while (bailout) {
		// Get user input
		prompt = getPrompt();
		char* reply = readline(prompt);
	
		/* if user quits, exit loop */
		if (!strcmp(reply, "quit")) {

			//Terminate the children
			terminate_process(tpid, index);		
			bailout = 0;
		} else { // Execute user command

			// 1. Parse the user input contained in reply
			if ((token = strtok(reply," ")) == NULL) {

				//No Command
				printf("RSI: Enter a command\n");
				continue;
			} 
			//Set command equal to the first parsed input
			command = token;

			//Create a string which is the user's input parsed by spaces
			i = 0;
			do 
			{
				arg[i++] = token;
			} while((token = strtok(NULL, " ")) != NULL);
			//Parse the &
			if (!strcmp(arg[i-1], "&")) {
				background = 1;
				arg[i-1] = NULL;
			} else {
				background = 0;
				arg[i] = NULL;
			}

			// 2. If "cd", then change directory by chdir()
			if(!strcmp(command, "cd")) {
				//change directory by chdir() 
				rv = chdir(arg[1]);
				if (rv != 0) {
					//Cd error
					printf("RSI: cd: %s\n", strerror(errno));
					rv = 0;
				}
			} else {
				// 3. Else, execute command by fork() and exec()

				//Create a string that contains all the user input parsed by spaces				
    			childpid = fork();
				if (childpid >= 0) /* fork succeeded */
				{
					if (childpid == 0) /* fork() returns 0 to the child process */
					{
						//Set execvp to rv for error checking
						rv = execvp(command, arg);
						if (rv != 0) {

							//Change error message to command not found
							if (errno == ENOENT) {	
								printf("RSI: %s: command not found\n", command);
							} else {
								perror("RSI");
							}
							rv = 0;
						}
						exit(0);
					} else {
						if (background == WNOHANG) {
							tpid[index++] = childpid;
							waitpid(childpid, 0, WNOHANG);	
						} else {
							waitpid(childpid, 0, 0);
						}
					}
				} else {
					//Fork Failed
					perror("RSI: Failed to Fork\n");
				}
			}
		}
	}
	printf("RSI:  Exiting normally.\n");
	return(0);
}
