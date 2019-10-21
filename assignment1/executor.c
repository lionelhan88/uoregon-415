#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "executor.h"


char *command, *pram1, *pram2;


void commandExecutor(char *input){


	
	command = strtok(input, " \n");

	pram1 = strtok(NULL, " \n");
	pram2 = strtok(NULL, " \n");
	
	while(command != NULL){

		if(strstr(command, "ls") || strstr(command, "pwd")){
			
			if(pram1 != NULL){
				printf("Error! Incorrect Syntax. No control found\n");
			}else if(strstr(command, "ls" )){
				listDir();
			}else if(strstr(command, "pwd")){
				showCurrentDir();
				}

		}else if(strstr(command, "mkdir") || strstr(command, "rm") || strstr(command, "cd")
			|| strstr(command, "cat") ){

			if(pram2 != NULL ){
				printf("Error! Incorrect Syntax. No control found\n");
			}else if(strstr(command, "mkdir") && strlen(command) == 5){
				makeDir(pram1);
			}else if(strstr(command, "cd") && strlen(command) == 2){
				changeDir(pram1);
			}else if(strstr(command, "rm") && strlen(command) == 2){
				deleteFile(pram1);
			}else if(strstr(command, "cat") && strlen(command) == 3 ){
				displayFile(pram1);
			}
		}else{

			if(strstr(command, "cp")  && pram1 != NULL && pram2 != NULL){
				copyFile(pram1,	pram2);
			}else if(strstr(command, "mv") && pram1 != NULL && pram2 != NULL){
				moveFile(pram1, pram2);
			}else if(pram2 == NULL){
				printf("cp: missing destination file operand after %s\n", pram1);
			}
		}
		command = strtok(NULL, " ");

	}

}