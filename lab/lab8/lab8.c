/*=============================================================================
 * Program Name: lab8
 * Author: Jared Hall
 * Date: 11/17/2019
 * Description:
 *     A simple program that implements a queue of meal tickets
 *
 * Notes:
 *     1. DO NOT COPY-PASTE MY CODE INTO YOUR PROJECTS.
 *===========================================================================*/

//========================== Preprocessor Directives ==========================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//=============================================================================

//================================= Constants =================================
#define MAXNAME 15
#define MAXQUEUES 4
#define MAXTICKETS 3
#define MAXDISH 20
//=============================================================================

//============================ Structs and Macros =============================
typedef struct mealTicket{
	int ticketNum;
	char *dish;
} mealTicket;

typedef struct MTQ {
	char name[MAXNAME];
	struct mealTicket * const buffer;
	int head;
	int tail;
	const int length;
} MTQ;

#define MTQ_DEF(var,id)                                                       \
	mealTicket var##_buf[MAXTICKETS+1] = {[MAXTICKETS].ticketNum = -1};       \
	MTQ var = {                                                               \
	.name = id,                                                               \
	.buffer = var##_buf,                                                      \
	.head = 0,                                                                \
	.tail = 0,                                                                \
	.length = MAXTICKETS,                                                     \
	}
	

MTQ *registry[MAXQUEUES];
int TICKET = 1;
MTQ_DEF(breakfast, "Breakfast");
MTQ_DEF(lunch, "Lunch");
MTQ_DEF(dinner, "Dinner");
MTQ_DEF(bar, "Bar");
//=============================================================================

//================================= Functions =================================
int enqueue(char *MTQ_ID, mealTicket *MT) {
	int ret = 0; 
	int i, flag = 0;
	
	//Step-1: Find registry
	for(i=0;i<MAXQUEUES;i++) { 
		if(strcmp(MTQ_ID, registry[i]->name) == 0) { flag = 1; break; }
	}

	//STEP-2: Enqueue the ticket
	if(flag) {
		int tail = registry[i]->tail;
		if(registry[i]->buffer[tail].ticketNum != -1) {
			MT->ticketNum = TICKET;
			TICKET++;
			registry[i]->buffer[tail] = *MT;
			if(tail == registry[i]->length) { registry[i]->tail = 0; }
			else { registry[i]->tail++; }
			ret = 1;
		}
	}
	return ret;
}

int dequeue(char *MTQ_ID, int ticketNum, mealTicket *MT) {
	int ret = 0;
	int i, flag = 0;
	
	//Step-1: Find registry
	for(i=0;i<MAXQUEUES;i++) { 
		if(strcmp(MTQ_ID, registry[i]->name) == 0) { flag = 1; break; }
	}
	
	//Step-2: Dequeue the ticket
	if(flag) {
		int head = registry[i]->head;
		int tail = registry[i]->tail;
		
		if(head != tail && ticketNum == registry[i]->buffer[head].ticketNum) {
			//copy the ticket
			MT->ticketNum = registry[i]->buffer[head].ticketNum;
			strcpy(MT->dish, registry[i]->buffer[head].dish);
			
			//change the null ticket to empty
			if(head == 0) { 
				registry[i]->buffer[registry[i]->length].ticketNum = 0;
			} else { 
			registry[i]->buffer[head-1].ticketNum = 0;
			}
			
			//change the current ticket to null
			registry[i]->buffer[head].ticketNum = -1;
			
			//increment the head
			if(head == registry[i]->length+1) { registry[i]->head = 0; }
			else { registry[i]->head++; }
			ret = 1;
		}
	}
	return ret;
}
//=============================================================================

//=============================== Program Main ================================
int main(int argc, char argv[]) {
	//Variables Declarations
	char *qNames[] = {"Breakfast", "Lunch", "Dinner", "Bar"};
	char *bFood[] = {"Eggs", "Bacon", "Steak"};
	char *lFood[] = {"Burger", "Fries", "Coke"};
	char *dFood[] = {"Steak", "Greens", "Pie"};
	char *brFood[] = {"Whiskey", "Sake", "Wine"};
	int i, j, t = 1;
	int test[4];
	char dsh[] = "Empty";
	mealTicket bfast[3] = {[0].dish = bFood[0], [1].dish = bFood[1], [2].dish = bFood[2]};
	mealTicket lnch[3] = {[0].dish = lFood[0], [1].dish = lFood[1], [2].dish = lFood[2]};
	mealTicket dnr[3] = {[0].dish = dFood[0], [1].dish = dFood[1], [2].dish = dFood[2]};
	mealTicket br[3] = {[0].dish = brFood[0], [1].dish = brFood[1], [2].dish = brFood[2]};
	mealTicket ticket = {.ticketNum=0, .dish=dsh};
	
	//STEP-1: Initialize the registry
	registry[0] = &breakfast;
	registry[1] = &lunch;
	registry[2] = &dinner;
	registry[3] = &bar;
	
	//STEP-2: Push meal tickets into the queues
	for(i=0;i<MAXTICKETS;i++) {enqueue(qNames[0], &bfast[i]);}
	for(i=0;i<MAXTICKETS;i++) {enqueue(qNames[1], &lnch[i]);}
	for(i=0;i<MAXTICKETS;i++) {enqueue(qNames[2], &dnr[i]);}
	for(i=0;i<MAXTICKETS;i++) {enqueue(qNames[3], &br[i]);}

	//STEP-3: Dequeue in round robin fashion
	printf("========== Dequeuing Tickets ==========\n");
	for(i=0; i<MAXTICKETS; i++) {
		if(i != 0) {t = i + 1;}
		for(j=0; j<MAXQUEUES; j++) {
			dequeue(qNames[j], t, &ticket);
			printf("Queue: %s - Ticket Number: %i - Dish: %s\n", qNames[j], ticket.ticketNum, ticket.dish);
			t = t + 3;
		}
		printf("\n");
	}
	printf("=======================================\n\n");
	
	//Step-4: Testing
	printf("========== Testing ==========\n");
	TICKET = 1;
	test[0] = dequeue(qNames[0], TICKET, &ticket);
	test[3] = enqueue(qNames[0], &bfast[0]);
	enqueue(qNames[0], &bfast[0]); 
	enqueue(qNames[0], &bfast[0]);
	test[2] = enqueue(qNames[0], &bfast[0]);
	test[1] = dequeue(qNames[0], 1, &ticket);

	if(test[0]) {printf("Test Case: A - Result: Success\n");}
	else {printf("Test Case: A - Result: Fail\n");}
	if(test[1]) {printf("Test Case: B - Result: Success\n");}
	else {printf("Test Case: B - Result: Fail\n");}
	if(test[2]) {printf("Test Case: C - Result: Success\n");}
	else {printf("Test Case: C - Result: Fail\n");}
	if(test[3]) {printf("Test Case: D - Result: Success\n");}
	else {printf("Test Case: D - Result: Fail\n");}
	printf("=============================\n");
	return EXIT_SUCCESS;
}
//=============================================================================