#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


pid_t forkChild(char *command, char **arguments){
	pid_t pid; 

	pid = fork();
	if(pid == -1){
		perror("fork");			
	}else if(pid == 0){

		//printf("child pid is %d\n", getpid());	
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
}

static int alarmFlg = 0;
void alarmHandler(int signal){

	alarmFlg=1;

}


void MCP_Stats(pid_t _pid){
	    //get the info of this requested PID from /proc
    char file[1000];
    sprintf(file, "/proc/%u/stat", _pid);

    //see if we got info, if so open it or return
    FILE *fp = fopen(file, "r");
    if (fp == NULL) return;

    int pidl, utime, stime;
    char name[1000], state;
    //grab only what we're looking for below, and ignore the rest
    fscanf(fp, "%d %s %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d", &pidl, name, &state, &utime, &stime);
    //we are going to print the pid, the name of the pid, the state the pid
    //is in (S, Z, T, etc.), and the total time. each tabbed
    printf("[%d] | %s | %c | %d\n", pidl, name, state, utime + stime);

    //close file
    fclose(fp);


}



int main(int argc, char *argv[]){

	size_t size, line;
	char *text= NULL;
	char* token = NULL;
	pid_t pid[10];
	FILE *fp;
	int count=0, i, j=0, wstatus, countArg = 0, k, a=0, commands;

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

		for(i=0; i<j; i++){
			printf("suspending pid %d\n", pid[i]);
			kill(pid[i], SIGSTOP);
		}
		system("sleep 2");
		int runPid, exist[j], a=1, b=0;
		pid_t w;

		for(int i=0; i<j; i++){
			exist[i] = 0;
		}

		signal(SIGALRM, alarmHandler);	
		kill(pid[0],SIGCONT);
	//	MCP_Stats(pid[0]);
		w = waitpid(pid[0], &wstatus, WNOHANG);
		while(a){
			for(runPid=0; runPid<j; runPid++){
				alarm(1);
			 	system("sleep 2");
			 	if(w == 0 ){
			 		MCP_Stats(pid[runPid]);
			 		if(alarmFlg==1){
			 			
			 			//printf("stopping signal to %d\n", pid[runPid]);		
						kill(pid[runPid], SIGSTOP);							// sending stop signal
						
						//printf("running signal to %d\n", pid[(runPid+1)%j]);
						kill(pid[(runPid+1)%j],SIGCONT);					// sending start signal
						MCP_Stats(pid[(runPid+1)%j]);
						
						alarmFlg = 0 ;
			 		}

						
				}else if(w == -1 ){
					exist[b] = -1;											// respectively put -1 when pid is finished running 
					b++;
				}
				w = waitpid(pid[(runPid+1)%j], &wstatus, WNOHANG);
			}
			for(int x=0; x<j; x++){
				if(exist[x] == 0){											// if any of element in exist is 0 keep while loop
					a=1;
				}else{
					a=0;													// exit while loop when all elements are -1
				}
			} 
		}		
		for(k=0; k<j; k++){
			printf("pid is waiting %d\n", pid[k]);
		 	waitpid(pid[k], &wstatus, WEXITED );
		}
		
	}
	free(text);
	fclose(fp);
	
	return 0;
}
