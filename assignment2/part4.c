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

static int alarmFlg = 0;
void alarmHandler(int signal){
    alarmFlg=1;
}

//Reference of proc

//https://stackoverflow.com/questions/33266678/how-to-extract-info-in-linux-with-a-c-code-from-proc
//https://linux.die.net/man/5/proc
//https://www.redhat.com/archives/axp-list/2001-January/msg00363.html
//https://stackoverflow.com/questions/1420426/how-to-calculate-the-cpu-usage-of-a-process-by-pid-in-linux-from-c
void print_status(pid_t _pid){

    //get the info of this requested PID from /proc
    char tmp[1000];
    sprintf(tmp, "/proc/%u/stat", _pid);


    FILE *filename = fopen(tmp, "r");
    if (filename == NULL) return;

    int pid_num, stime, utime;
    char command[1000], state;
    unsigned long vsize;
 
    fscanf(filename, "%d %s %c %lu %lu %lu", &pid_num, command, &state, &utime, &stime, &vsize);

    printf("command = %s   state = %c  stime = %lu   utime = %lu  execution time = %lu  vsize = %lu\n", 
    command , state, stime, utime, utime + stime , vsize);//print the reading command
    //The filename of the executable, in parentheses. 
    //This is visible whether or not the executable is swapped out.
    //printf("state = %c\n", state);
    //print the state of child process
    //printf("pid = %d\n", pid_num);
    //printf("stime = %lu\n", stime);
    //print stime:Amount of time that this process has been scheduled
    //in kernel mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).
    //printf("utime = %lu\n", utime);
    //printf("execution time = %lu\n", utime + stime);
    //print the execution time
    //printf("vsize = %lu\n", vsize);
    //print the Virtual memory size in bytes.

    //close file
    fclose(filename);
}

int main(int argc, char *argv[]) {
    FILE *input = NULL;
    //char buf[1024];
    size_t nread;
    char *line;
    size_t bufsize = 1024;
    int count_line = 0;
    int k;
    const char *space = " ";
    char *token;
    char **ptr;
    size_t ptrSize = 20;

    if (argc == 2) {  
        input = fopen(argv[1], "r");
        if (!input) {//error handling
            printf("%s not exiting\n", argv[1]);
            exit(1);
        }
    }

    line = (char *)malloc(bufsize * sizeof(char));
    while((nread = getline(&line, &bufsize, input)) != -1){
        count_line += 1;
    }
    free(line);
    fclose(input);

    line = (char *)malloc(bufsize * sizeof(char));
    pid_t pid[count_line];
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
            sa.sa_handler = sighandler; //set sighandler to sa struct
            sigemptyset(&sa.sa_mask); //empty and clean the set
            sa.sa_flags = 0; //set flag as default
            sigaction(SIGALRM, &sa, NULL); //
            //printf("the pid is %d\n", getpid());
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

    int w, wstatus;

    for(int k = 0 ; k < count_line; k++){
        printf("stop pid %d\n", pid[k]);
        kill(pid[k], SIGSTOP); //send SIGINT to terminate the child
    }

    for(int k=0; k<count_line; k++){
        w = waitpid(pid[k], &wstatus, WNOHANG);
    }

    kill(pid[0], SIGCONT);
    printf("     The RUNNING/CHECKING pid is: %d\n", pid[0]);
    printf(" -------------------------------------------------\n");
    printf("\n");
    int runPid = 0;
    int check2 = 1;
    while(check2){
        signal(SIGALRM,alarmHandler);
        alarm(3);
        sleep(3);
        if(alarmFlg == 1){
            while(1){
                if((w = waitpid(pid[runPid%count_line], &wstatus, WNOHANG)) == 0){
                    printf("     The STOPING pid is: %d\n", pid[runPid%count_line]);
                    kill(pid[runPid%count_line],SIGSTOP);
                    runPid++;
                    break;
                }else{
                    printf("     The %d have done\n", pid[runPid%count_line]);
                    // index++;
                    break;
                }               
            }

            while(1){
                if((w = waitpid(pid[runPid%count_line], &wstatus, WNOHANG)) == 0){
                    printf("     The RUNNING/CHECKING  pid is: %d\n", pid[runPid%count_line]);
                    kill(pid[runPid%count_line],SIGCONT);
                    sleep(1);
                    for(int checkindex = 0; checkindex < count_line; checkindex++){
                        if(waitpid(pid[checkindex], &wstatus, WNOHANG) == 0){
                            print_status(pid[checkindex]); //print the running process status
                        }
                    }
                    break;
                }
                runPid++;
            }
            alarmFlg = 0;
        }
        printf(" -------------------------------------------------\n");
        
        for(int i = 0 ; i < count_line; i++){
            if((w = waitpid(pid[i], &wstatus, WNOHANG)) == 0){
                check2 = 1;
            }else{
                check2 = 0;
            }   
        }          
    }

    printf("All child processes joined. Exiting\n");
    fclose(input);

    return 0;
}

void sighandler(int signum)
{   
    
}