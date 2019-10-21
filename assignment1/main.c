#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "command.c"
#include <ctype.h>
#include "executor.h"
#include "executor.c"

int main(int argc, char* argv[]){

	char *token;
	int input;
	char* text=NULL;
	size_t size=0, line; 
	char *dummy=NULL;
	setbuf(stdout, NULL);

	if (strcmp(argv[1],"-command")==0){
		printf (">>> ");
		input = getline(&text, &size, stdin);											// take input from user
		dummy = text;

		while(input != -1){																// keep taking input until exit was input
			token = strtok_r(dummy, ";", &dummy);
			if(strcmp(token, "exit\n") == 0){
				free(text);
				exit(1);
			}
			while(token != NULL){
				while (strlen(token) < 2){												// when encounter invalid input
					printf("Error! Unrecognized command\n");							// keep taking input from user
					printf(">>> ");		
					getline(&text, &size, stdin);
					printf("\n");
					dummy = text;
					token = strtok_r(dummy, ";", &dummy);								// split command again
				} 
				if (strcmp(token,"exit\n") == 0 ){										// eixt program when 
					free(text);
					exit(1);
				}
				commandExecutor(token);
				token = strtok_r(dummy, ";", &dummy);
			}
			
			printf (">>> ");
			input = getline(&text, &size, stdin);
			dummy = text;
		}
		free(text);

	}else if(strstr(argv[2],".txt")){

		FILE * fp;
		fp = fopen(argv[2], "r");
		while( (line = getline(&text, &size, fp)) != -1){
			dummy = text;
			while(token = strtok_r(dummy, ";", &dummy)){
				commandExecutor(token);
				fflush(stdout);
			}
		}
		token = NULL;
		fclose(fp);
	}		
	free(text);
	
	return 0;
}



