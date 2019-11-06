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

		sleep(4);
		_exit(0);
	}
	return pid;
}



int main(int argc, char *argv[]){

	size_t size, line;
	char* temp = NULL;
	char *text= NULL;
	char* token = NULL;
	pid_t pid[10];
	FILE *fp;
	int count=0, i, j=0, wstatus, countArg = 0, k, numCom = 0, commands;

	if(argc != 2){
		printf("The program need exact one file to execute, please try again\n");
		exit(0);
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
			
			numCom += 1; 
		
		 	pid[numCom] = forkChild(array[0], array);				// fork child processes
		 	if(pid[numCom] < 0){
		 		//free(text);
		 		//fclose(fp);
		 		exit(-1);
		 	}
			//freeArray(array, count);
			count = 0;
			countArg = 0;
			j++;
			
		}

		sleep(3);

		for(int k=0; k<j; k++){
			printf("pid is waiting %d\n", pid[k]);
		 	waitpid(pid[k], &wstatus, WUNTRACED);
		}

		
		
		
	}
	free(text);
	fclose(fp);
	
	return 0;
}
