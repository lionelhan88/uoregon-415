all: psudo-shell
	
psudo-shell: main.o 
	gcc -o psudo-shell main.o 

psudo-shell.o: main.c 
	gcc -c -std=c99 main.c 
	
clean:
	rm -f core *.o psudo-shell	
