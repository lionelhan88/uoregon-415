#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h> 


pid_t forkChild(char *command, char **arguments){
	pid_t pid;

	pid = fork();
	if(pid == -1){
		perror("fork\n");
	}else if(pid == 0){

		printf("child pid is %d\n", getpid());
		execvp( command, arguments);

		sleep(4);
		_exit(0);
	}
	return pid;
}

int main(int argc, char *argv[]){

	size_t size, line;
	char *text= NULL;
	char* token = NULL;
	pid_t pid[10];
	FILE *fp;
	int count=0, i,k, j=0, wstatus, countArg = 0, numCom = 0;

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

			char *array[3];											// initialize an array pointer
			for(i=0; i <= count ; i++){
				array[i] = NULL;
			}

			token = strtok(text, " \r\n");
			for(i=0; i < count ; i++){
				array[i] = token;									// store token into the array pointer
				token = strtok(NULL, " \r\n");
			}

		 	pid[numCom] = forkChild(array[0], array);				// fork child processes
		 	if(pid[numCom] == -1){
		 		free(text);
		 		fclose(fp);
		 		_exit(-1);
		 	}
		 	numCom += 1;

			count = 0;
			countArg = 0;
			j++;
		}
		sleep(3);
		for(k=0; k<j; k++){
			printf("pid is waiting %d\n", pid[k]);
		 	waitpid(pid[k], &wstatus, WUNTRACED);
		}

		free(text);
		fclose(fp);
	}



	return 0;
}
