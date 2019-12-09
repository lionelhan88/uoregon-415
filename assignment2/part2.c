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

void sighandler(int);

int main(int argc, char *argv[]) {
    //initial the varibales.
    FILE *input = NULL;
    char buf[1024];
    size_t nread;
    char *line;
    size_t bufsize = 1024;
    int count_line = 0;
    int k;
    const char *space = " ";
    char *token;
    char **ptr;
	size_t ptrSize = 20;

    if (argc == 2) {  //open the input file
        input = fopen(argv[1], "r");
        if (!input) {//error handling
            printf("%s not exiting\n", argv[1]);
            exit(1);
        }
    }

    line = (char *)malloc(bufsize * sizeof(char));
    while((nread = getline(&line, &bufsize, input)) != -1){
    	count_line += 1; //count the line number in the input file
    }
    free(line);
    fclose(input);


    //open the file again
    line = (char *)malloc(bufsize * sizeof(char));
    pid_t pid[count_line], w;
    int wstatus;
    input = fopen(argv[1], "r");

    for(int index = 0 ; index < count_line; index++){
	    ptr = (char **)malloc(ptrSize * sizeof(char*));
        memset(ptr, 0, sizeof(ptr));
	 	getline(&line, &bufsize, input);

	 	// *ptr = (char *)malloc(count_line * sizeof(char));
	 	strtok(line , "\n");
	 	int arg_index = 1;
	 	token = NULL;
	 	token = strtok(line, space);
	 	ptr[0] = (char*)malloc(strlen(token) + 1);
	 	strcpy(ptr[0],token);
	 	//printf("%s\n", ptr[0]);
	 	token = strtok(NULL, space);
	 	while(token != NULL){
	 		//printf("%s\n", token);
	 		ptr[arg_index] = malloc(strlen(token)+1);
	 		//printf("123 %s\n", ptr[arg_index]);
	 		strcpy(ptr[arg_index++],token);
	 		token = strtok(NULL, space);
	 	}
        ptr[arg_index++] = NULL;
    	pid[index] = fork();
		if(pid[index] < 0){
			printf("failed to fork\n");
		}
    	if(pid[index] == 0){
            struct sigaction sa; //initial the sigaction struct
            sa.sa_handler = sighandler; //set handler 
            sigemptyset(&sa.sa_mask); //empty and clean the set 
            sa.sa_flags = 0;  // set the flag as default
            sigaction(SIGUSR1, &sa, NULL); //send the SIGUSR1
            sigset_t sigSet; //initial set for signal
            sigemptyset(&sigSet); //empty all sig in sigset
    		printf("the pid is %d\n", getpid());
            sigaddset(&sigSet, SIGUSR1);
            //sleep(3);
            int sig;
            sigwait(&sigSet, &sig);
            printf("Child Process: %d  Received SIGUSR1 signal\n", getpid());
            fclose(input);
            free(line);
    		execvp(ptr[0],ptr);
            for(k = 0; k < (arg_index); k++){ //free the ptr array
                free(ptr[k]);
            }
            free(ptr);
    		_exit(-1);
    	}
    	for(k = 0; k < (arg_index); k++){
    		free(ptr[k]);
    	}
    	free(ptr);
    }

    free(line);

    sleep(3);
    for(int in = 0 ; in < count_line; in++){
        if(kill(pid[in], SIGUSR1) != 0){ //send the SIGUSR1 second time to sighandler
            printf("failed to send SIGUSR1 signal to pid: %d\n", pid[in]);
        }
    }
    sleep(5);
    for(int k = 0 ; k < count_line; k++){
        if(kill(pid[k], SIGSTOP) != 0){ //send the SIGSTOP second time to sighandler
            printf("failed to send SIGSTOP signal to pid: %d\n", pid[k]);
        }
        else{
            printf("%d Received SIGSTOP signal\n", pid[k]);
        }
    }

    printf("\n");
    sleep(5);
    for(int l = 0 ; l < count_line; l++){
        if(kill(pid[l], SIGCONT) != 0){ //send the SIG second time to sighandler
            printf("failed to send SIGCONT signal to pid: %d\n", pid[l]);
        }
        else{
            printf("%d Received SIGCONT signal\n", pid[l]);
        }
    }

    sleep(3);
    for(int m = 0 ; m < count_line; m++){
       //kill(pid[m], SIGINT);
        w = waitpid(pid[k], &wstatus , WUNTRACED);
    }


    fclose(input);
    printf("All child processes joined. Exiting\n");
    return 0;
}

void sighandler(int signum)
{   
    //do nothing but will let pid wait 
}