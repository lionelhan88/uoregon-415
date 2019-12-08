all: main.exe
	
main.exe: main.o 
	gcc -pthread -o main.exe main.o 

main.o: main.c 
	gcc -c main.c 
	
clean:
	rm -f core *.o main.exe
