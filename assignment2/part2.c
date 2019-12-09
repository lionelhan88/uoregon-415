#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h>


void myHandler(int mySignal){

	// printf("Child Process: %d waiting - Received signal: %d\n", getpid(),mySignal);
	// sigset_t set;
	// sigemptyset(&set);
	// sigaddset(&set, SIGUSR1);
	// sigprocmask(SIG_SETMASK, &set, NULL);
	// int sig;
	// sigwait(&set, &sig);
}


pid_t forkChild(char *command, char **arguments){
	pid_t pid;
	struct sigaction act;
	act.sa_handler = &myHandler;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);

	pid = fork();
	if(pid == -1){
		printf("Error, failed to fork \n");
	}else if(pid == 0){

		printf("child pid is %d\n", getpid());
		sigaction(SIGUSR1,&act,NULL);
		execvp( command, arguments);
		sleep(2);
		_exit(0);
	}
	return pid;
}

void singler(int count, int signal, pid_t *pid){
	int i;
	for(i=1; i<=count; i++){
		printf("child receiving signal %d\n", signal);
		kill(pid[i], signal);
	}
}

int main(int argc, char *argv[]){

	size_t size, line;
	char *text= NULL;
	char* token = NULL;
	pid_t pid[10];
	FILE *fp;
	int count=0, i, j=0, k, wstatus, countArg = 0, a=0;

	if(argc != 2){
		printf("The program need exact one file to execute, please try again\n");
		exit(-1);
	}

	if(strstr(argv[1], ".txt")){

		fp = fopen(argv[1], "r");
		while((line = getline(&text, &size, fp)) != -1){			// while loop

			for(i=0; i<strlen(text); i++){
				if(text[i] == ' '){
	 				countArg++;
 				}
			}
			count = countArg + 1; 									// count how many arguments each line has in the file

			char *array[count];
			for(i=0; i <= count ; i++){
				array[i] = NULL;									// initialize an array pointer
			}

			token = strtok(text, " \r\n");
			for(i=0; i < count ; i++){
				array[i] = token;									// store token into the array pointer
				token = strtok(NULL, " \r\n");
			}

		 	pid[a] = forkChild(array[0], array);					// fork child processes
		 	a++;
			count = 0;
			countArg = 0;
			j++;
		}
		
		sleep(3);
		printf("sending SIGUSR1 signal to pause all the processes!\n");		// sigusr signal to pause all the children
    	singler(j, SIGUSR1, pid);
   		sleep(3);
   		printf("sending SIGSTOP signal to stop all the processes!\n");
   		singler(j, SIGSTOP, pid);
   		sleep(4);
   		printf("sending SIGCONT signal to continue all the processes!\n");
   		singler(j, SIGCONT, pid);
		sleep(3);
		

		for(k=0; k<j; k++){
		 	waitpid(pid[k], &wstatus, WUNTRACED);
			}

	}
	free(text);
	fclose(fp);

	return 0;
}
