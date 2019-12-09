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


pid_t forkChild(char *command, char **arguments){
	pid_t pid; 

	pid = fork();
	if(pid == -1){
		perror("fork");			
	}else if(pid == 0){
		printf("child pid is %d\n", getpid());	
		execvp( command, arguments);
		system("sleep 2");
		_exit(0);
	}
	return pid;
}

void singler(int count, int signal, pid_t *pid){
	int i;
	printf("recieved signal %d\n", signal);
	for(i=1; i<=count; i++){
		kill(pid[i], signal);
	}
	printf("--------------------------------------------------------------------------------\n");
}

static int alarmFlg = 0;
void alarmHandler(int signal){

	alarmFlg=1;

}


void MCP_Stats(pid_t _pid){
	   
    char file[1000];
    sprintf(file, "/proc/%u/stat", _pid);

    FILE *fp = fopen(file, "r");
    if (fp == NULL) return;

    int pid, utime, stime, ppid;
    long signal, blocked, flags;
    char name[1000], state;
    fscanf(fp, "%d %s %c %*d %*d %*d %*lu %*lu %*lu", 
    	&pid, name, &state, &ppid, &flags, &utime, &stime, &signal);

    printf("[%d] | %s | state: %c | flags: %lu | time: %lu | signal: %lu \n", 
    	pid, name, state, flags, utime + stime, signal);								// take 
    printf("--------------------------------------------------------------------------------\n");
    fclose(fp);
}


int main(int argc, char *argv[]){

	size_t size, line;
	char *text= NULL;
	char* token = NULL;
	pid_t pid[10];
	FILE *fp;
	int count=0, i, j=0, wstatus, countArg = 0, k, a=0, commands,x;

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
			count = countArg + 1; 
			char *array[count];										// count how many arguments each line has in the file
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

		system("sleep 1");
		for(i=0; i<j; i++){
			printf("suspending pid %d\n", pid[i]);
			kill(pid[i], SIGSTOP);
		}
		printf("--------------------------------------------------------------------------------\n");
		system("sleep 2");
		int runPid=0,  a=1, b=0;
		pid_t w;

		kill(pid[0], SIGCONT);
		while(a){
			signal(SIGALRM,alarmHandler);
			
			alarm(2);
			system("sleep 2");
			if(alarmFlg==1){
				w = waitpid(pid[runPid%j], &wstatus, WNOHANG);
				while(1){
					if((w = waitpid(pid[runPid%j], &wstatus, WNOHANG)) == 0){					// sending stop signal to current running child
						printf("stopping signal to %d\n", pid[runPid%j]);
						printf("--------------------------------------------------------------------------------\n");
						kill(pid[runPid%j], SIGSTOP);	
						runPid++;
						break;
					}else{
						printf("Child is finished %d\n", pid[runPid%j]);
						printf("--------------------------------------------------------------------------------\n");
						break;
					}
				}

				while(1){
					if((w = waitpid(pid[runPid%j], &wstatus, WNOHANG)) == 0){					// sending run signal to next alive child
						printf("running signal to %d\n", pid[runPid%j]);
						printf("--------------------------------------------------------------------------------\n");
						kill(pid[runPid%j], SIGCONT);	
						system("sleep 2");
						 for(i = 0; i < j; i++){
	                        if(waitpid(pid[i], &wstatus, WNOHANG) == 0){
	                            MCP_Stats(pid[i]); //print the running process status
	                        }
	                    }
						break;
					}
					runPid++;
				}
				alarmFlg = 0 ;
		}
		for( i = 0 ; i < j; i++){																// check if there is any child alive
            if((w = waitpid(pid[i], &wstatus, WNOHANG)) == 0){
                a = 1;
            }else{
                a = 0;
            }   
        }
	}}
	free(text);
	fclose(fp);
	
	return 0;
}
