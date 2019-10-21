#include "command.h"
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>


void listDir(){ /*for the ls command*/
	
	DIR *dir;
	struct dirent *dirent;
	dir = opendir(".");

	if(dir == NULL){	
		write(2, "Error! Unable to open directory. \n", 35);						// empty directory
		return;
	}
	while((dirent = readdir(dir)) != NULL){										

		write(1, dirent->d_name, strlen(dirent->d_name));							// ls command
		write(1,"\n",1);
	}
	closedir(dir);

}

void showCurrentDir(){ /*for the pwd command*/

	char directory[100];
	write(1, getcwd(directory, sizeof(directory)), 									// pwd command
		strlen(getcwd(directory, sizeof(directory))));
	write(1,"\n",1);
					
}

void makeDir(char *dirName){ /*for the mkdir command*/

	if(dirName == NULL){															// missing operand
		write(1, "mkdir: missing operand\n", 23);
	}else{
		mkdir(dirName,0777);														// mkdir command
	}
}

void changeDir(char *dirName){ /*for the cd command*/
	
	DIR *dir;
	struct dirent *dirent;
	dir = opendir(".");
	int check = 1;

	if(dir == NULL){																// make sure the current directory is not empty
		write(2, "Error! Unable to open directory. \n", 35);
		return;
	}
	while((dirent = readdir(dir)) != NULL){											// execute cd command with valid input
		if(strcmp(dirent->d_name,dirName) == 0 || strcmp(dirName, "..") == 0 
			|| strcmp(dirName, ".") == 0){

			chdir(dirName);
			check = 0;		
			break;
		}
	}
	if(check == 1){																	// directory not found
		write(1,"No such directory found\n", 24);
	}	
	closedir(dir);

}


void copyFile(char *sourcePath, char *destinationPath){ /*for the cp command*/
	char *content, *targetFile = NULL;
	size_t size = 1048;
	int socFile, dstFile, line, closeSocFile, closeDstFile;
	content = (char *)malloc(size * sizeof(char));
	socFile = open(sourcePath, O_RDONLY, S_IRUSR);								// open source file
	dstFile = open(destinationPath, O_WRONLY | O_CREAT, S_IRUSR);				// open target file

	if(socFile != -1 && dstFile != -1){											// destination path is also a file and in the same directory
																				// read source file 
		line = read(socFile, content, sizeof(content));		
		while(line > 0){
			write(dstFile, content, line);										// cope to the target file
			line = read(socFile, content, sizeof(content));	
		}

		close(socFile);
		close(dstFile);
		free(content);

	}else if(socFile == -1){													// source cannot be found
		write(2, "cp : cannot stat", 16);
		write(2, sourcePath, sizeof(sourcePath));
		write(2, ": No such file or directory \n", 29);
		return;
	}else{																		// when destination path is another directory
		char current_Destination[200];
		getcwd(current_Destination, sizeof(current_Destination));				// record current path
		line = read(socFile, content, sizeof(content));							// read from source file
		chdir(destinationPath);													// switch to the target directory
		targetFile = strrchr(sourcePath,'/') + 1;								// create target file
		dstFile = open(targetFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		while(line > 0){
			write(dstFile, content, line);										// cope to the target file
			line = read(socFile, content, sizeof(content));	
		}
		write(1,"\n",1);
												
		chdir(current_Destination);												// go back to the current director
	
		close(socFile);
		close(dstFile);
		free(content);
	}
	

}


void moveFile(char *sourcePath, char *destinationPath){ /*for the mv command*/

	rename(sourcePath,destinationPath);

}


void deleteFile(char *filename){ /*for the rm command*/
	
	int file; 
	file = open(filename, O_RDONLY, S_IRUSR);	

	if(file == -1){
		write(2, "rm: cannot remove", 17);
		write(2, filename, strlen(filename));
		write(2, ": No such file or directory\n", 28);
		return;
	}else{
		unlink(filename);
	}
}


void displayFile(char *filename){ /*for the cat command*/

	int file, line; 
	size_t size = 1024;
	char *content = (char *)malloc(size * sizeof(char));

	file = open(filename, O_RDONLY, S_IRUSR);	

	if(file == -1){
		write(2, "cat: ", 5);
		write(2, filename, strlen(filename));
		write(2, ": No such file or directory\n", 28);
		return;
	}else{
		
		line = read(file, content, sizeof(content));
		
		while(line > 0){
	
			write(1, content, line);
			line = read(file, content, sizeof(content));	
		}	
		write(1,"\n", 1);	
		free(content);
		return;
	}
}