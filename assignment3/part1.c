#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#define MAXENTRIES 10									// max number of entries 
#define DIFF 5											// variable to clean up the old entries
#define NUMOFBUFFER 5									// max number of queues 

int enNum = 1;

struct topicEntry{
	int entryNum;
	struct timeval timeStamp;
	int pubID;
	char photoURL[100];
	char photoCaption[100];
};

struct topicQueue{
	char name[200];
	struct topicEntry *entry;
	int head;
	int tail;
	int length ;
};

struct pthread{
	int flag;
	pthread_t thread_id;
};

struct threadEnq{
		struct topicEntry topicEntry, temp;
		char Q_id[100];
		struct timeval timeStamp;
		int result;
		int lockPos;
		int lastEntry;
};

#define RCBUFFER(tpQueue, id, len)						\
	struct topicEntry tpQueue##_entry[MAXENTRIES];		\
	struct topicQueue tpQueue = {						\
		.name = id,										\
		.entry = tpQueue##_entry,						\
		.head = 0,										\
		.tail = 0,										\
		.length = len									\
	}



struct pthread pub[MAXENTRIES], sub[MAXENTRIES], cle[NUMOFBUFFER];
struct topicQueue *Registry[NUMOFBUFFER];
struct threadEnq threadEnq[MAXENTRIES], threadDeq[MAXENTRIES], threadCle[NUMOFBUFFER];
pthread_mutex_t mutex[NUMOFBUFFER] = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock[NUMOFBUFFER] = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int enqueue(struct topicEntry newEntry, char *Q_id){

	for(int i=0; i<NUMOFBUFFER; i++){
		if (strcmp(Registry[i]->name, Q_id) == 0) {
			int headNum = Registry[i]->head - Registry[i]->tail;
			int j=0;	
			if(headNum == MAXENTRIES){
				return 0;
			}
			for (int k = Registry[i]->tail; k < Registry[i]->head; ++k){
				Registry[i]->entry[j] = Registry[i]->entry[k];
				j++;
			}
			if (Registry[i]->tail != 0){
				Registry[i]->tail = 0;
			}
			Registry[i]->head = headNum;
			Registry[i]->entry[headNum] = newEntry;
			Registry[i]->entry[headNum].entryNum = enNum;
			sleep(2);
			gettimeofday(&Registry[i]->entry[headNum].timeStamp,NULL);
			headNum += 1;
			Registry[i]->head = headNum;
			enNum += 1;
			Registry[i]->length += 1;
			return 1;
		}	
	}	
} // enqueue()


int getEntry( struct topicEntry *temp, char *Q_id, int lastEntry){

	if(lastEntry == 0){
		return 0;
	}
	for(int i=0; i<NUMOFBUFFER; i++){
		if (strcmp(Registry[i]->name, Q_id) == 0) {
			if(Registry[i]->head == Registry[i]->tail || lastEntry == Registry[i]->head){
				return 0;
			}
			for (int j = Registry[i]->tail; j < Registry[i]->head; j++){
				if (Registry[i]->entry[j].entryNum == lastEntry){
					temp->entryNum = Registry[i]->entry[(j+1)].entryNum;
					temp->timeStamp = Registry[i]->entry[(j+1)].timeStamp;
					temp->pubID = Registry[i]->entry[(j+1)].pubID;
					strcpy(temp->photoURL, Registry[i]->entry[(j+1)].photoURL);
					strcpy(temp->photoCaption, Registry[i]->entry[(j+1)].photoCaption);
					return 1;
				}else if(lastEntry <= Registry[i]->tail){
					temp->entryNum = Registry[i]->entry[Registry[i]->tail].entryNum;
					temp->timeStamp = Registry[i]->entry[Registry[i]->tail].timeStamp;
					temp->pubID = Registry[i]->entry[Registry[i]->tail].pubID;
					strcpy(temp->photoURL, Registry[i]->entry[Registry[i]->tail].photoURL);
					strcpy(temp->photoCaption, Registry[i]->entry[Registry[i]->tail].photoCaption);
					return temp->entryNum;
				}
			}
		}
	}	
	return 0;
} // getEntry()

int dequeue(char *Q_id){

	for(int i=0; i<NUMOFBUFFER; i++){
		if (strcmp(Registry[i]->name, Q_id) == 0) {
			if(Registry[i]->head == Registry[i]->tail){
				return 0;
			}
			Registry[i]->tail += 1;
			return 1;
		}
	}
} // dequeue()

int cleanUp(struct topicEntry *temp, char *Q_id){
	gettimeofday(&temp->timeStamp,NULL);
	int diff;
	for(int i=0; i<NUMOFBUFFER; i++){
		
		if (strcmp(Registry[i]->name, Q_id) == 0) {
		
			if (Registry[i]->head == 0){
				return 0;
			}
			for (int j = 0; j < Registry[i]->head; ++j){
				diff = (int)(temp->timeStamp.tv_sec) - (int)(Registry[i]->entry[j].timeStamp.tv_sec);
				
				if (diff > DIFF){
					dequeue(Registry[i]->name);
				}
			}
			return 1;
		}
	}	
} // cleanUp()


void initThreadEnq(int lockPos, char *Q_id){
	
	for (int i = 0; i < MAXENTRIES; ++i){
		threadEnq[i].lockPos = lockPos;
		strcpy(threadEnq[i].Q_id, Q_id);
		threadEnq[i].result = 0;
	}
} // initialize threadEnq

void initThreadDeq(int lockPos, char *Q_id){
	
	for (int i = 0; i < MAXENTRIES; ++i){
		threadDeq[i].lockPos = lockPos;
		strcpy(threadDeq[i].Q_id, Q_id);
		threadDeq[i].lastEntry = (i+1);
		threadDeq[i].result = 0;
	}
} // initialize threadDeq

void initThreadCle(int number, char *Q_id){
	
	for (int i = 0; i < number; ++i){
		threadCle[i].lockPos = i;
		strcpy(threadCle[i].Q_id, Q_id);
		threadCle[i].result = 0;
	}
} // initialize threadDeq

void* publisher(void* arg){
	struct threadEnq *threadEnq =(struct threadEnq *) arg;
	pthread_mutex_lock(&mutex[threadEnq->lockPos]); 
	threadEnq->result = enqueue(threadEnq->topicEntry, threadEnq->Q_id);
	printf("publisher result: %d\n", threadEnq->result);
	pthread_mutex_unlock(&mutex[threadEnq->lockPos]);
	for (int i = 0; i < MAXENTRIES; ++i){
		if (pub[i].thread_id == pthread_self()){
			pub[i].flag = 0;
		}
	}
	return NULL;
} // publisher

void* subscriber(void* arg){
	struct threadEnq *threadDeq =(struct threadEnq *) arg;
	pthread_mutex_lock(&mutex[threadDeq->lockPos]);
	threadDeq->result = getEntry(&threadDeq->temp, threadDeq->Q_id, threadDeq->lastEntry);
	printf("-------------------------------------------------------------------------\n");
	printf("subscriber result: %d\n", threadDeq->result);
	printf("threadEnq: %s %s\n", threadDeq->temp.photoURL, threadDeq->temp.photoCaption);
	printf("-------------------------------------------------------------------------\n");
	pthread_mutex_unlock(&mutex[threadDeq->lockPos]);
	for (int i = 0; i < MAXENTRIES; ++i){
		if (sub[i].thread_id == pthread_self()){
			sub[i].flag = 0;
		}
	}
	return NULL;
} // subscriber

void* pthread_cleanUp(void* arg){
	struct threadEnq *threadCle =(struct threadEnq *) arg;
	int diff, check = 1; 
	pthread_mutex_unlock(&lock[threadCle->lockPos]);
	threadCle->result = cleanUp(&threadCle->temp, threadCle->Q_id);
	printf("clean up result: %d\n", threadCle->result);
	
	pthread_mutex_unlock(&lock[threadCle->lockPos]);
	for (int i = 0; i < NUMOFBUFFER; ++i){
		if (cle[i].thread_id == pthread_self()){
			cle[i].flag = 0;
		}
	}
//	sched_yield();
	return NULL;
} // pthread_cleanUp()



int main(){
	int result;
	struct topicEntry newEntry1, newEntry2, newEntry3, newEntry4, newEntry5, 
		newEntry6, newEntry7, newEntry8, newEntry9, newEntry10, temp;
	char *qName = "first_queue";
	// struct topicQueue *tpQueue;

	RCBUFFER(tpQueue, qName, 5);

	// RCBUFFER(tpQueue1, "second_queue");
	// RCBUFFER(tpQueue2, "third_queue");
	// RCBUFFER(tpQueue3, "forth_queue");
	// RCBUFFER(tpQueue4, "fifth_queue");
	Registry[0] = &tpQueue;
	// Registry[1] = &tpQueue1;
	// Registry[2] = &tpQueue2;
	// Registry[3] = &tpQueue3;
	// Registry[4] = &tpQueue4;
	// initRgistry( 0, qName, 5);
	// printf("%s, %d\n", Registry[0]->name, Registry[0]->length);
	
	newEntry1.pubID = 1;
	strcpy(newEntry1.photoURL, "test photo URL 1");
	strcpy(newEntry1.photoCaption, "test photo caption 1");

	newEntry2.pubID = 2;
	strcpy(newEntry2.photoURL, "test photo URL 2");
	strcpy(newEntry2.photoCaption, "test photo caption 2");	

	newEntry3.pubID = 3;
	strcpy(newEntry3.photoURL, "test photo URL 3");
	strcpy(newEntry3.photoCaption, "test photo caption 3");	

	newEntry4.pubID = 4;
	strcpy(newEntry4.photoURL, "test photo URL 4");
	strcpy(newEntry4.photoCaption, "test photo caption 4");	

	newEntry5.pubID = 5;
	strcpy(newEntry5.photoURL, "test photo URL 5");
	strcpy(newEntry5.photoCaption, "test photo caption 5");	

	newEntry6.pubID = 6;
	strcpy(newEntry6.photoURL, "test photo URL 6");
	strcpy(newEntry6.photoCaption, "test photo caption 6");	

	newEntry7.pubID = 7;
	strcpy(newEntry7.photoURL, "test photo URL 7");
	strcpy(newEntry7.photoCaption, "test photo caption 7");

	newEntry8.pubID = 8;
	strcpy(newEntry8.photoURL, "test photo URL 8");
	strcpy(newEntry8.photoCaption, "test photo caption 8");	

	newEntry9.pubID = 9;
	strcpy(newEntry9.photoURL, "test photo URL 9");
	strcpy(newEntry9.photoCaption, "test photo caption 9");	

	newEntry10.pubID = 10;
	strcpy(newEntry10.photoURL, "test photo URL 10");
	strcpy(newEntry10.photoCaption, "test photo caption 10");	

	printf("--------------------  Single Thread  --------------------\n");						// single thread test case

	printf("Test Case: clean up empty queue\n");												// clean up empty queue
	result = cleanUp(&temp, qName);
	printf("Result: %d, cannot remove entries from empty queue\n", result);
	printf("---------------------------------------------------------\n");
	result = getEntry(&temp, qName, 1);													// get entry empty queue
	printf("Test Case: get last entry from empty queue \n");
	printf("Result: %d\n", result);
	printf("---------------------------------------------------------\n");
	sleep(1);
	result = enqueue(newEntry1, qName);													// enqueue to non-empty queue
	printf("Test Case: enqueue to empty queue \n");
	printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	printf("---------------------------------------------------------\n");
	result = enqueue(newEntry2, qName);												// enqueue to non-empty queue
	printf("Test Case: enqueue to non-empty queue \n");
	printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	printf("---------------------------------------------------------\n");
	sleep(1);
	result = enqueue(newEntry3, qName);												// enqueue to non-empty queue
	printf("Test Case: enqueue to non-empty queue \n");
	printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	printf("---------------------------------------------------------\n");
	sleep(1);
	result = enqueue(newEntry4, qName);												// enqueue to non-empty queue
	printf("Test Case: enqueue to non-empty queue \n");
	printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	printf("---------------------------------------------------------\n");
	sleep(1);
	result = enqueue(newEntry5, qName);												// enqueue to non-empty queue
	printf("Test Case: enqueue to non-empty queue \n");
	printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	printf("---------------------------------------------------------\n");
	sleep(1);

	printf("\n");
	printf("Test Case: clean up the queue, remove old topic entries \n");				// clean up entries from the queue
	cleanUp(&temp, qName);
	printf("\n");
	printf("The queue has %d entries after clean up\n", (Registry[0]->head - Registry[0]->tail));
	// printf("\n");
	// for (int i = Registry[0]->tail; i < Registry[0]->head  ; ++i){
	// 	printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 		Registry[0]->entry[i].photoURL, Registry[0]->entry[i].photoCaption, (int)(Registry[0]->entry[i].timeStamp.tv_sec), 
	// 		Registry[0]->entry[i].pubID, Registry[0]->entry[i].entryNum);
	// 	printf("---------------------------------------------------------\n");
	// }
	// printf("\n");	
	// printf("Test Case: get last entry 4 from non-empty queue while entry 5 is in the queue\n"); 
	// result = getEntry(&temp, qName, 4);												// get entry from none empty queue
	// printf("Result: %d\n", result);
	// printf("\n");
	// printf("The lastEntry+1 is: \n");
	// printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 	temp.photoURL, temp.photoCaption,(int)(temp.timeStamp.tv_sec), temp.pubID, temp.entryNum);
	// printf("---------------------------------------------------------\n");
	// printf("Test Case: get last entry 2 from non-empty queue while entry 2 is not in the queue\n");
	// result = getEntry(&temp, qName, 2);												// get entry from none empty queue
	// printf("Result: %d\n", result);
	// printf("\n");
	// printf("The lastEntry+1 is: \n");
	// printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 	temp.photoURL, temp.photoCaption,(int)(temp.timeStamp.tv_sec), temp.pubID, temp.entryNum);
	// printf("---------------------------------------------------------\n");
	// sleep(1);
	// result = enqueue(newEntry6, qName);												// enqueue to non-empty queue
	// printf("Test Case: enqueue to non-empty queue \n");
	// printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	// printf("---------------------------------------------------------\n");
	// sleep(1);
	// result = enqueue(newEntry7, qName);												// enqueue to non-empty queue
	// printf("Test Case: enqueue to non-empty queue \n");
	// printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	// printf("---------------------------------------------------------\n");
	// sleep(1);
	// result = enqueue(newEntry8, qName);												// enqueue to non-empty queue
	// printf("Test Case: enqueue to non-empty queue \n");
	// printf("Result: %d, Head: %d, Tail: %d\n", result, Registry[0]->head, Registry[0]->tail);
	// printf("---------------------------------------------------------\n");
	// printf("The queue has %d entries\n", (Registry[0]->head - Registry[0]->tail));				// print out the entire queue
	// printf("\n");
	// for (int i = 0; i < Registry[0]->head; ++i){
	// 	printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 		Registry[0]->entry[i].photoURL, Registry[0]->entry[i].photoCaption, (int)(Registry[0]->entry[i].timeStamp.tv_sec), 
	// 		Registry[0]->entry[i].pubID, Registry[0]->entry[i].entryNum);
	// 	printf("---------------------------------------------------------\n");
	// }


	

	// // printf("\n");
	// // printf("--------------------  Multi Thread  --------------------\n");

	// threadEnq[1].topicEntry.pubID = newEntry2.pubID;
	// strcpy(threadEnq[1].topicEntry.photoURL, newEntry2.photoURL);
	// strcpy(threadEnq[1].topicEntry.photoCaption, newEntry2.photoCaption);

	// threadEnq[2].topicEntry.pubID = newEntry3.pubID;
	// strcpy(threadEnq[2].topicEntry.photoURL, newEntry3.photoURL);
	// strcpy(threadEnq[2].topicEntry.photoCaption, newEntry3.photoCaption);
	
	// threadEnq[3].topicEntry.pubID = newEntry4.pubID;
	// strcpy(threadEnq[3].topicEntry.photoURL, newEntry4.photoURL);
	// strcpy(threadEnq[3].topicEntry.photoCaption, newEntry4.photoCaption);
	
	// threadEnq[4].topicEntry.pubID = newEntry5.pubID;
	// strcpy(threadEnq[4].topicEntry.photoURL, newEntry5.photoURL);
	// strcpy(threadEnq[4].topicEntry.photoCaption, newEntry5.photoCaption);
	
	// threadEnq[0].topicEntry.pubID = newEntry1.pubID;
	// strcpy(threadEnq[0].topicEntry.photoURL, newEntry1.photoURL);
	// strcpy(threadEnq[0].topicEntry.photoCaption, newEntry1.photoCaption);
	
	// threadEnq[5].topicEntry.pubID = newEntry6.pubID;
	// strcpy(threadEnq[5].topicEntry.photoURL, newEntry6.photoURL);
	// strcpy(threadEnq[5].topicEntry.photoCaption, newEntry6.photoCaption);
	
	// threadEnq[6].topicEntry.pubID = newEntry7.pubID;
	// strcpy(threadEnq[6].topicEntry.photoURL, newEntry7.photoURL);
	// strcpy(threadEnq[6].topicEntry.photoCaption, newEntry7.photoCaption);
	
	// threadEnq[7].topicEntry.pubID = newEntry8.pubID;
	// strcpy(threadEnq[7].topicEntry.photoURL, newEntry8.photoURL);
	// strcpy(threadEnq[7].topicEntry.photoCaption, newEntry8.photoCaption);
	
	// threadEnq[8].topicEntry.pubID = newEntry9.pubID;
	// strcpy(threadEnq[8].topicEntry.photoURL, newEntry9.photoURL);
	// strcpy(threadEnq[8].topicEntry.photoCaption, newEntry9.photoCaption);
	
	// threadEnq[9].topicEntry.pubID = newEntry10.pubID;
	// strcpy(threadEnq[9].topicEntry.photoURL, newEntry10.photoURL);
	// strcpy(threadEnq[9].topicEntry.photoCaption, newEntry10.photoCaption);
		
	// initThreadEnq(0, "first_queue");
	// initThreadDeq(0, "first_queue");
	// initThreadCle(5, "first_queue");

	// for (int i = 0; i < MAXENTRIES; ++i){
	// 	if ( pub[i].flag == 0 ){
	// 		pub[i].flag = 1;
	// 		pthread_create(&(pub[i].thread_id), NULL, publisher, &threadEnq[i]);
	// 	}
	// }

	// for (int i = 0; i < MAXENTRIES; ++i){
	// 	if ( sub[i].flag == 0 ){
	// 		sub[i].flag = 1;
	// 		pthread_create(&(sub[i].thread_id), NULL, subscriber, &threadDeq[i]);
	// 	}
	// }

	// // for (int i = 0; i < NUMOFBUFFER; ++i){
	// // 	if ( cle[i].flag == 0 ){
	// // 		cle[i].flag = 1;
	// // 		pthread_create(&(cle[i].thread_id), NULL, pthread_cleanUp, &threadCle[i]);
	// // 	}
	// // }

	// for (int k = 0; k < MAXENTRIES; ++k){	
	// 	if (pub[k].flag == 1){
	// 		pthread_join(pub[k].thread_id, NULL);
	// 	}
	// }

	// for (int k = 0; k < MAXENTRIES; ++k){	
	// 	if (sub[k].flag == 1){
	// 		pthread_join(sub[k].thread_id, NULL);
	// 	}
	// }

	// // for (int k = 0; k < NUMOFBUFFER; ++k){	
	// // 	if (cle[k].flag == 1){
	// // 		pthread_join(cle[k].thread_id, NULL);
	// // 	}
	// // }

	// // for (int i = 0; i < NUMOFBUFFER; ++i)
	// // {
	// // 	printf("asdfasdf %s\n", Registry[i]->name);
	// // }

	// for (int i = 0; i < Registry[0]->length; ++i){
	// 	printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 		Registry[0]->entry[i].photoURL, Registry[0]->entry[i].photoCaption, (int)(Registry[0]->entry[i].timeStamp.tv_sec), 
	// 		Registry[0]->entry[i].pubID, Registry[0]->entry[i].entryNum);
	// 	printf("---------------------------------------------------------\n");
	// }
	// printf("Dequeueing from the empty queue\n");
	// threadEnq1.topicEntry.entryNum = 1;
	// threadEnq1.topicQueue = &tpQueue;
	// pthread_create(&tid[1], NULL, pthread_dequeue, &threadEnq1);
	// pthread_join(tid[1], NULL);

	// printf("Adding new entry into queue \n");
	// pthread_create(&tid[0], NULL, pthread_enqueue, &threadEnq1);
	// pthread_join(tid[0], NULL);
	// pthread_create(&tid[0], NULL, pthread_enqueue, &threadEnq2);
	// pthread_join(tid[0], NULL);
	// pthread_create(&tid[0], NULL, pthread_enqueue, &threadEnq3);
	// pthread_join(tid[0], NULL);
	// pthread_create(&tid[0], NULL, pthread_enqueue, &threadEnq4);
	// pthread_join(tid[0], NULL);
	// pthread_create(&tid[0], NULL, pthread_enqueue, &threadEnq5);
	// pthread_join(tid[0], NULL);

	// printf("\nThe queue now has %d entries\n\n", tpQueue.head);
	// for (int i = 0; i < tpQueue.head; ++i){
	// 	printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 		tpQueue.entry[i].photoURL, tpQueue.entry[i].photoCaption, (int)(tpQueue.entry[i].timeStamp.tv_sec), 
	// 		tpQueue.entry[i].pubID, tpQueue.entry[i].entryNum);
	// 	printf("---------------------------------------------------------\n");
	// }

	// printf("Dequeueing from the non-empty queue\n");
	// threadEnq1.topicEntry.entryNum = 1;
	// threadEnq1.topicQueue = &tpQueue;
	// pthread_create(&tid[1], NULL, pthread_dequeue, &threadEnq1);
	// pthread_join(tid[1], NULL);
	// printf("\nThe queue now has %d entries\n\n", tpQueue.head);
	// for (int i = 0; i < tpQueue.head; ++i){
	// 	printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 		tpQueue.entry[i].photoURL, tpQueue.entry[i].photoCaption, (int)(tpQueue.entry[i].timeStamp.tv_sec), 
	// 		tpQueue.entry[i].pubID, tpQueue.entry[i].entryNum);
	// 	printf("---------------------------------------------------------\n");
	// }

	// threadEnq1.topicEntry.entryNum = 1;
	// threadEnq1.temp = &temp; 
	// threadEnq1.topicQueue = &tpQueue;
	// printf("Get entry from the non-empty queue, last entry is: %d\n", threadEnq1.topicEntry.entryNum);
	// pthread_create(&tid[2], NULL, pthread_getEntry, &threadEnq1);
	// pthread_join(tid[2], NULL);
	
	// printf("\nPhoto URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 		temp.photoURL, temp.photoCaption, (int)(temp.timeStamp.tv_sec), 
	// 		temp.pubID, temp.entryNum);
	// printf("---------------------------------------------------------\n");

	
	// sleep(2);
	// gettimeofday(&threadEnq1.timeStamp,NULL);;
	// threadEnq1.topicQueue = &tpQueue;
	// printf("Cleaning up the old entries from the queue\n");
	// pthread_create(&tid[3], NULL, pthread_cleanUp, &threadEnq1);
	// pthread_join(tid[3], NULL);
	// printf("\nThe queue now has %d entries\n\n", tpQueue.head);
	// for (int i = 0; i < tpQueue.head; ++i){
	// 	printf("Photo URL: %s\nPhotot Caption: %s\nTime Stamp: %d\npubID: %d\nentryNum: %d\n",  
	// 		tpQueue.entry[i].photoURL, tpQueue.entry[i].photoCaption, (int)(tpQueue.entry[i].timeStamp.tv_sec), 
	// 		tpQueue.entry[i].pubID, tpQueue.entry[i].entryNum);
	// 	printf("---------------------------------------------------------\n");
	// }


	return 0;
} // main()
