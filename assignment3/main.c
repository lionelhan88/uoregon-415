#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#define MAXENTRIES 500									// max number of entries 											
#define NUMOFBUFFER 500									// max number of queues
int delta = 0;											// delta variable to clean up the old entries
int enNum = 1;
int check = 1 , subFile=0, subTable=0, z=-1;
int i, j;


struct topicEntry{
	int entryNum;
	struct timeval timeStamp;
	int pubID;
	char photoURL[100];
	char photoCaption[100];
	char topName[100];
};

struct topicQueue{
	char name[200];
	struct topicEntry *entry;
	int head;
	int tail;
	int length ;
	int id;
};

struct pthread{
	int flag;
	pthread_t thread_id;
};

struct threadEnq{
		struct topicEntry topicEntry;
		char Q_id[100];
		struct timeval timeStamp;
		int result;
		int lockPos;
		int lastEntry;
		int len;
		int regisNum;
};

struct topicQueue Registry[NUMOFBUFFER];
struct topicEntry tepp[MAXENTRIES], temp1;
struct topicEntry Entries[NUMOFBUFFER][MAXENTRIES];
struct threadEnq threadEnq[MAXENTRIES], threadDeq[MAXENTRIES], threadCle[NUMOFBUFFER];
struct pthread pub[MAXENTRIES], sub[MAXENTRIES], cle;

pthread_mutex_t mutex[NUMOFBUFFER] = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock[NUMOFBUFFER] = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t condition_mutex;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int enqueue(struct topicQueue *tpQueue, char *URL, char *Caption, int pubID){
	
	int headNum = (tpQueue->head - tpQueue->tail);
	int j=0;

	if(headNum == MAXENTRIES){												// return 0 if the queue is empty
		return 0;
	}

	for (int k = tpQueue->tail; k < tpQueue->head; ++k){					// reset the head
		tpQueue->entry[j] = tpQueue->entry[k];
		j++;
	}

	if (tpQueue->tail != 0){												// reset the tail
		tpQueue->tail = 0;
	}

	tpQueue->head = headNum;												//----------------------//
	tpQueue->entry[headNum].pubID = pubID;									//	  	                //
	strcpy(tpQueue->entry[headNum].photoURL, URL);							//	                    //
	strcpy(tpQueue->entry[headNum].photoCaption, Caption);					//	add entry data      //
	tpQueue->entry[headNum].entryNum = enNum;								//	into the queue      //
	gettimeofday(&tpQueue->entry[headNum].timeStamp,NULL);					//						//
	headNum += 1;															//						//
	tpQueue->head = headNum;												//						//
	enNum += 1;																//----------------------//
	return 1;
} // enqueue()


int getEntry(struct topicQueue *tpQueue, int lastEntry, struct topicEntry *temp){
	if(tpQueue->head == tpQueue->tail || 
		lastEntry == tpQueue->head || lastEntry == 0){						// return 0 if the queue is empty or the entry
		return 0;															// has been deleted
	}																		

	z+=1;
	for (int j = tpQueue->tail; j < tpQueue->head; j++){
		if (tpQueue->entry[j].entryNum == lastEntry){						// copy last entry +1 data to the temp buffer if
			tepp[z].entryNum = tpQueue->entry[(j+1)].entryNum;				// last entry is in the queue
			tepp[z].timeStamp = tpQueue->entry[(j+1)].timeStamp;
			tepp[z].pubID = tpQueue->entry[(j+1)].pubID;					// return 1 after it's done
			strcpy(tepp[z].topName, tpQueue->name);
			strcpy(tepp[z].photoURL, tpQueue->entry[(j+1)].photoURL);
			strcpy(tepp[z].photoCaption, tpQueue->entry[(j+1)].photoCaption);
			return 1;
		}else if(lastEntry <= tpQueue->tail){								// copy the closest entry data to the temp buffer
			tepp[z].entryNum = tpQueue->entry[tpQueue->tail].entryNum;		// if last entry isn't in the queue
			tepp[z].timeStamp = tpQueue->entry[tpQueue->tail].timeStamp;	// ie. if last entry is 1 and entry 1, 2,
			tepp[z].pubID = tpQueue->entry[tpQueue->tail].pubID;			// isn't in the queue, then take entry 3
			strcpy(tepp[z].topName, tpQueue->name);
			strcpy(tepp[z].photoURL, tpQueue->entry[tpQueue->tail].photoURL);
			strcpy(tepp[z].photoCaption, tpQueue->entry[tpQueue->tail].photoCaption);
			return tepp[z].entryNum;										// return the entry number 
		}
	}
	return 0;
} // getEntry()


int dequeue(struct topicQueue *tpQueue){

	if(tpQueue->head == tpQueue->tail){										// return 0 if the queue is empty			
		return 0;
	}
	tpQueue->tail += 1;														// move the tail towards to the head
	return 1;

} // dequeue()

int cleanUp(struct topicEntry *temp, struct topicQueue *tpQueue){
	gettimeofday(&temp->timeStamp,NULL);
	int diff, result;

	if (tpQueue->head == tpQueue->tail){									// return 0 if the queue is empty	
	
		return 0;
	}
	for (int j = 0; j < tpQueue->head; ++j){
		diff = (int)(temp->timeStamp.tv_sec) - 	
			(int)(tpQueue->entry[j].timeStamp.tv_sec);						// get time stamp, and compare it with each entry's delta
		if (diff > delta){
			result = dequeue(tpQueue);										// call dequeue function if the entry is too old
			printf("Dequeue is called, result %d\n", result);
			printf("Topic Queue head: %d, tail: %d\n", tpQueue->head, tpQueue->tail);
			printf("-------------------------------------------------------------------------\n");
			if (result == 0){
				return 0;
			}
		}
	}
	return 1;
} // cleanUp()

void initThreadEnq(char *Q_id){												// initialize struct that will be used
																			// in publisher
	for (i = 0; i < MAXENTRIES; ++i){
		threadEnq[i].lockPos = 0;
		strcpy(threadEnq[i].Q_id, Q_id);
		threadEnq[i].result = 0;
	}
} // initialize threadEnq


void initThreadDeq(char *Q_id){												// initialize struct that will be used
																			// in subscriber	
	for (i = 0; i < MAXENTRIES; ++i){
		threadDeq[i].lockPos = 0;
		strcpy(threadDeq[i].Q_id, Q_id);
		threadDeq[i].lastEntry = (i+1);
		threadDeq[i].result = 0;
	}
} // initialize threadDeq


void initPtred(){															// initialize struct that will be used
																			// for multi-thread						
	for (int i = 0; i < MAXENTRIES; ++i){
		pub[i].flag = 0;
		sub[i].flag = 0;
	}
} // initialize threadDeq


void* publisher(void* arg){													
	struct threadEnq *threadEnq =(struct threadEnq *) arg;
	pthread_mutex_lock(&condition_mutex);
	pthread_cond_wait(&cond, &condition_mutex);								// pthread conditional wait, waiting signal from main
	pthread_mutex_unlock(&condition_mutex);

	pthread_mutex_lock(&mutex[threadEnq->lockPos]); 						// lock the critical section for publisher
	threadEnq->result = enqueue(&Registry[threadEnq->regisNum], threadEnq->topicEntry.photoURL,
			threadEnq->topicEntry.photoCaption, threadEnq->topicEntry.pubID);
	printf("publisher result: %d, Topic Queue head: %d, tail: %d\n", threadEnq->result, Registry[threadEnq->regisNum].head,
			Registry[threadEnq->regisNum].tail);
	printf("-------------------------------------------------------------------------\n");
	pthread_mutex_unlock(&mutex[threadEnq->lockPos]);						// unlock the publisher critical section
	for (int i = 0; i < MAXENTRIES; ++i){
		if (pub[i].thread_id == pthread_self()){							// change the publisher thread flag
			pub[i].flag = 0;
			break;
		}
	}
	return NULL;
} // publisher


void* subscriber(void* arg){
	struct threadEnq *threadDeq =(struct threadEnq *) arg;
	pthread_mutex_lock(&condition_mutex);
	pthread_cond_wait(&cond, &condition_mutex);								// pthread conditional wait, waiting signal from main
	pthread_mutex_unlock(&condition_mutex);

	pthread_mutex_lock(&mutex[threadDeq->lockPos]);							// lock the critical section for subscriber
	threadDeq->result = getEntry(&Registry[threadEnq->regisNum], threadDeq->lastEntry, &temp1);
	printf("subscriber result: %d\n", threadDeq->result);
	printf("-------------------------------------------------------------------------\n");
	pthread_mutex_unlock(&mutex[threadDeq->lockPos]);						// unlock the subscriber critical section
	
	for (int i = 0; i < MAXENTRIES; ++i){
		if (sub[i].thread_id == pthread_self()){							// change the subscriber thread flag
			sub[i].flag = 0;
			break;
		}
	}
	return NULL;
} // subscriber

void* pthread_cleanUp(void* arg){
	struct threadEnq *threadCle =(struct threadEnq *) arg; 
	pthread_mutex_lock(&condition_mutex);	
	pthread_cond_wait(&cond, &condition_mutex);								// pthread conditional wait, waiting signal from main
	pthread_mutex_unlock(&condition_mutex);

	while(check == 1){
		pthread_mutex_lock(&lock[threadCle->lockPos]);						// lock the critical section for clean up
		check = cleanUp(&temp1, &Registry[threadCle->regisNum]);
		pthread_mutex_unlock(&lock[threadCle->lockPos]);					// unlock the subscriber critical section
		sched_yield();														// yeild to publisher and subscriber
	}
	return NULL;
} // pthread_cleanUp()

void initRgistry(){															// Registry initialization
	for (int i = 0; i < NUMOFBUFFER; ++i){
		Registry[i].head = 0;
		Registry[i].tail = 0;
		Registry[i].length = MAXENTRIES;
		Registry[i].entry = Entries[i];
		Registry[i].id = -1;
	}
}

void createQueue(int id, char *name, int len){								// Queue initialization
	strcpy(Registry[id].name, name);
	Registry[id].length = len;
	Registry[id].entry = Entries[id];
	Registry[id].id = id;
}

int main(int argc, char* argv[]){

	char *token, *pubToken, *subToken, *command[5], *pubcmd[5], *subcmd[4], *text = NULL, *dummy = NULL, pubFilename[100], 
		subFilename[100], *pubText = NULL, *subText = NULL, *dummy1 = NULL, *dummy2 = NULL, name[20] ;
	size_t size=0, line; 
	int tk1_cnt=0, tk2_cnt=0, tk3_cnt=0;
	int pub_cnt=0, pthred_cnt=0, sub_cnt=0, sthred_cnt=0;
	int actQueue=0, actPub=-1, cur = -1, ruc=-1, actSub=0, x=0, m;
	int publisherID[MAXENTRIES];
	struct topicEntry temp;
	int pQuery[MAXENTRIES], sQuery[MAXENTRIES];

	if(argc != 2){															// error handler for txt file missing
		printf("The program need one txt file to execute, please try again\n");
		printf("------------------------------------------------------------\n");
		exit(-1);
	}else if(strstr(argv[1],".txt")){
		initRgistry();
		initPtred();

		FILE *fp;
		fp = fopen(argv[1], "r");											// open and read input txt file
		
		while( (line = getline(&text, &size, fp)) != -1){
			dummy = text;
			while(token = strtok_r(dummy, " ", &dummy)){
				command[tk1_cnt] = token;
				tk1_cnt += 1;
			}
			tk1_cnt = 0;	
			if (strcmp(command[0], "create") == 0){							// "create topic" command
				printf("-------------------------------------------------------------------------\n");
				printf("Activating queue\n");
				createQueue(atoi(command[2]), command[3], atoi(command[4]));
				initThreadEnq(command[3]);
				initThreadDeq(command[3]);
				actQueue += 1;
			} 
			if(strcmp(command[0], "add") == 0){								// "add publisher/subscriber" command

				if (strcmp(command[1], "publisher") == 0){
					actPub+=1;												// varible that track act-publishers
					strcpy(pubFilename, strtok_r(command[2], "\"", &command[2]));
					if (strlen(pubFilename) <= 1 || !(strstr(pubFilename,".txt") ) ){
						printf("Publisher filename error, please try again\n");
						printf("-------------------------------------------------------------------------\n");
						fclose(fp);
						free(text);
						return -1;
					}else{
						FILE *fpPub;										// open publisher txt file 
						fpPub = fopen(pubFilename, "r");
						while( (line = getline(&pubText, &size, fpPub)) != -1){
							dummy1 = pubText;
							while(pubToken = strtok_r(dummy1, " ", &dummy1)){
								pubcmd[tk2_cnt] = pubToken;
								tk2_cnt += 1;
							}
							tk2_cnt = 0;
							if (strcmp(pubcmd[0], "put") == 0){				// "put" command
								cur = (atoi(pubcmd[1]) - 1);
								threadEnq[pub_cnt].topicEntry.pubID = cur;
								threadEnq[pub_cnt].regisNum = cur;
								strcpy(threadEnq[pub_cnt].topicEntry.photoURL, pubcmd[2]);
								strcpy(threadEnq[pub_cnt].topicEntry.photoCaption, pubcmd[3]);
								pub_cnt += 1;

							}else if(strcmp(pubcmd[0], "stop") == 0){		// "stop" command
								printf("Ends of publisher file: %s\n", pubFilename);
								printf("-------------------------------------------------------------------------\n");
								fclose(fpPub);							// close publisher txt file
								free(pubText);
								break;
							}else if(strcmp(pubcmd[0], "sleep") == 0){		// "sleep" command
								int sleepTime = atoi(pubcmd[1]) / 1000;
								sleep(sleepTime);
							}
						}

						for(i=0; i<actPub; i++){							// create thread for publisher
							if(pub[i].flag == 0){
								for(j=0; j<pub_cnt; j++){
									pub[i].flag = 1;
									threadEnq[j].topicEntry.pubID = pub[i].thread_id;
									pthread_create(&(pub[i].thread_id), NULL, publisher, &threadEnq[j]);
								}
								break;
							}

						}
					}
				}else if(strcmp(command[1], "subscriber") == 0){
					subFile += 1;
					strcpy(subFilename, strtok_r(command[2], "\"", &command[2]));
					if (strlen(subFilename) <= 1 || !(strstr(subFilename,".txt") ) ){
						printf("Subscriber filename error, please try again\n");
						printf("-------------------------------------------------------------------------\n");
						fclose(fp);
						free(text);
						return -1;
					}else{
						FILE *fpSub;
						fpSub = fopen(subFilename, "r");					// open subscriber txt file 
						while( (line = getline(&subText, &size, fpSub)) != -1){
							dummy2 = subText;
							while(subToken = strtok_r(dummy2, " ", &dummy2)){
								subcmd[tk3_cnt] = subToken;
								tk3_cnt += 1;
							}
							tk3_cnt = 0;
							if (strcmp(subcmd[0], "get") == 0){				// "get" command
								ruc = atoi(subcmd[1])-1;
								threadDeq[sub_cnt].topicEntry.pubID = ruc;
								threadDeq[sub_cnt].regisNum = ruc;
								threadDeq[sub_cnt].lastEntry = atoi(subcmd[2]);
								sub_cnt += 1;
								subTable += 1;

							}else if(strcmp(subcmd[0], "stop") == 0){		// "stop" command
								printf("Ends of subscribers file: %s\n", subFilename);
								printf("-------------------------------------------------------------------------\n");
								fclose(fpSub);								// close subscriber txt file
								free(subText);
								break;
							}else if(strcmp(subcmd[0], "sleep") == 0){		// "sleep" command
								int sleepTime = atoi(subcmd[1]) / 1000;
								sleep(sleepTime);
							}
						}

						sleep(1);
						for(i=0; i<actSub; i++){							// create thread for subscriber
							if(sub[i].flag == 0){
								for(j=0; j<sub_cnt; j++){
									sub[i].flag = 1;
									pthread_create(&(sub[i].thread_id), NULL, subscriber, &threadDeq[j]);
								}
								break;
							}
						}
					}
					m = subTable;
					subTable=0;
				}				
			} 

			if(strcmp(command[0], "query") == 0){							// "query" command
 				if(strcmp(command[1], "topics\n") == 0){
					printf("There are %d queues activating \n", actQueue );
					for (int i = 0; i < actQueue; ++i){
						printf("Queue id %d, name: %s, length: %d\n", Registry[i].id, Registry[i].name, Registry[i].length );
						printf("-------------------------------------------------------------------------\n");					 
					}
				}
				if (strcmp(command[1], "publishers\n") == 0){				// "query publishers" command
					for (int i = 0; i < MAXENTRIES; ++i){
						if(pub[i].flag == 1){
							printf("Proxy thread %ld - type: Publisher\n", pub[i].thread_id);
							printf("-------------------------------------------------------------------------\n");
						}	
					}
				}
				if(strcmp(command[1], "subscribers\n") == 0){				// "query subscribers" command
					for (int i = 0; i < MAXENTRIES; ++i){
						if(sub[i].flag == 1){
							printf("Proxy thread %ld - type: Subscriber \n", sub[i].thread_id);
							printf("-------------------------------------------------------------------------\n");
						}	
					}
				}
			} 

			if(strcmp(command[0], "delta") == 0){							// "delta" command
				delta = atoi(command[1]);
				printf("delta is %d\n", delta);
				printf("-------------------------------------------------------------------------\n");
			}

			 if(strcmp(command[0], "start\n") == 0 || strcmp(command[0], "start") == 0){			// "start" command

			 	for (int k = 0; k < MAXENTRIES; ++k){	
					if (pub[k].flag == 1){								// join publiser thread
						pthread_mutex_lock(&condition_mutex);
						pthread_cond_broadcast(&cond);
						pthread_mutex_unlock(&condition_mutex);
						pthread_join(pub[k].thread_id, NULL);
					}
				}

				sleep(2);
				for (int k = 0; k < MAXENTRIES; ++k){					// join subscriber thread
					if (pub[k].flag == 1){
						pthread_mutex_lock(&condition_mutex);
						pthread_cond_broadcast(&cond);
						pthread_mutex_unlock(&condition_mutex);
						pthread_join(sub[k].thread_id, NULL);

					}
				}

				sleep(1);
				int o = 0, v=0;											// write data to subscriber html file
				for(i=0; i<subFile; i++){
					x += 1;
					snprintf(name, 12, "Sub_%d.html", x);
					FILE *ptrFile = fopen( name, "w");
					for(j=0; j<m; j++){
						fprintf(ptrFile, "<!DOCTYPE html>\n"); 
						fprintf(ptrFile, "<html>\n"); 
						fprintf(ptrFile, "<title>%s</title>\n", name);
						fprintf(ptrFile, "\n"); 
						fprintf(ptrFile, "<style>\n");
						fprintf(ptrFile, "table, th, td {\n");
						fprintf(ptrFile, "  border: 1px solid black;\n");
						fprintf(ptrFile, "  border-collapse: collapse;\n");
						fprintf(ptrFile, "}\n");
						fprintf(ptrFile, "th, td {\n");
						fprintf(ptrFile, "  padding: 5px;\n");
						fprintf(ptrFile, "}\n");
						fprintf(ptrFile, "th {\n");
						fprintf(ptrFile, "  text-align: left;\n");
						fprintf(ptrFile, "}\n");
						fprintf(ptrFile, "</style>\n");
						fprintf(ptrFile, "\n");
						fprintf(ptrFile, "</head>\n");
						fprintf(ptrFile, "<body>\n");
						fprintf(ptrFile, "\n");
						fprintf(ptrFile, "<h1>Subsriber: %s</h1>\n", name);
						fprintf(ptrFile, "\n");
						fprintf(ptrFile, "<h2>Topic Name: %s</h2>\n", tepp[v].topName);
						fprintf(ptrFile, "\n");
						fprintf(ptrFile, "<table style=\"width:100%%\" align=\"middle\">\n");
						fprintf(ptrFile, " <tr>\n");
						fprintf(ptrFile, "  <th>CAPTION</th>\n");
						fprintf(ptrFile, "  <th>PHOTO-URL</th>\n");
						fprintf(ptrFile, " </tr>\n");
						fprintf(ptrFile, " <tr>\n");
						fprintf(ptrFile, "  <td>%s</td>\n", tepp[v].photoCaption);
						fprintf(ptrFile, "  <td>%s</td>\n", tepp[v].photoURL);
						fprintf(ptrFile, "</table>\n");
						fprintf(ptrFile, "</body>\n");
						fprintf(ptrFile, "</html>\n");
						v += 1;

					}
					
				}

				for (int i = 0; i < actQueue; ++i){									// create clean up thread

                	threadCle[i].regisNum = i;
                    pthread_create(&(cle.thread_id), NULL, pthread_cleanUp, &threadCle[i]);
                }

				sleep(1);
		
				pthread_mutex_lock(&condition_mutex);
				pthread_cond_broadcast(&cond);
				printf("Start cleaning up the old entries\n");						// join clean up thread
				printf("-------------------------------------------------------------------------\n");
				pthread_mutex_unlock(&condition_mutex);
				pthread_join(cle.thread_id, NULL);			 	

			}		
		}		
		token = NULL;
		fclose(fp);																	// close input txt file
	}		

	free(text);
	return 0;
} // main()
