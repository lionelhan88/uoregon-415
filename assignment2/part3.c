#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


pid_t forkChild(char *command, char **arguments){
	pid_t pid;

	pid = fork();
	if(pid == -1){
		printf("Error, failed to fork \n");
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
}

static int alarmFlg = 0;
void alarmHandler(int signal){

	alarmFlg=1;

}

int exist_test(int exist[], int count){
	int alive=0;
	for(int x=0; x<count; x++){
		if(exist[x] == 0){											// if any of element in exist is 0 keep while loop
		//	printf("exist[%d] is %d\n",x, exist[x]);
			alive += 1;
		}
	}
	return alive;
}

int main(int argc, char *argv[]){

	size_t size, line;
	char *text= NULL;
	char* token = NULL;
	pid_t pid[10];
	FILE *fp;
	int test = 0;
	int count=0, i, j=0, wstatus, countArg = 0, k, a=0;

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
		system("sleep 1");
		int runPid, exist[j], a=1, b=0;
		pid_t w;

		for(int i=0; i<j; i++){
			exist[i] = 0;
		}

		signal(SIGALRM, alarmHandler);
		kill(pid[0],SIGCONT);
		w = waitpid(pid[0], &wstatus, WNOHANG);
		a = exist_test(exist, j);
		while(a>1){
			for(runPid=0; runPid<j; runPid++){
				printf("process %d is running \n", pid[runPid]);
			 	if(w == 0){
			 		alarm(1);
			 		system("sleep 1");
			 		if(alarmFlg==1){
			 			printf("stopping signal to %d\n", pid[runPid]);
						kill(pid[runPid], SIGSTOP);							// sending stop signal
						printf("running signal to %d\n", pid[(runPid+1)%j]);
						kill(pid[(runPid+1)%j],SIGCONT);					// sending start signal
						alarmFlg = 0 ;
			 		}

				}else if(w == -1 ){
					exist[runPid] = -1;											// respectively put -1 when pid is finished runningHa
				}
			 	test = (runPid + 1) % j;
				w = waitpid(pid[test], &wstatus, WNOHANG);
				a = exist_test(exist, j);
				printf("w here is %d, for pid: %d\n", w, pid[(runPid+1)%j]);
			}
		}

		//w = waitpid(pid[runPid], &wstatus, WNOHANG);
		if(w==0){
			printf("running last available process %d\n", pid[runPid]);
			kill(pid[runPid],SIGCONT);
		}
		for(k=0; k<j; k++){
			printf("pid is waiting %d\n", pid[k]);
		 	waitpid(pid[k], &wstatus, WNOHANG );
		}

	}
	free(text);
	fclose(fp);

	return 0;
}
