#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct mealTicket{
	int ticketNum;
	char *dish;
};

struct MTQ{
	char *name;
	struct mealTicket *buffer;
	int head;
	int tail;
	int length;
};


int enqueue(char *MTQ_ID, struct mealTicket *MT, struct MTQ *meal){
	int ticNum = 0;

	while(meal->buffer[ticNum].dish != NULL){
	// 	printf("ticket num is %d\n", ticNum);
	 	ticNum += 1; 
	 	if(ticNum == 4){
	 		ticNum = 0;
	 		break;
	 	}
	 
	}
	if(MTQ_ID != meal->name){
		printf("Meal name not match!\n");
		return 0;
	}else{
		meal->buffer[ticNum].dish = MT->dish;
		meal->buffer[ticNum].ticketNum = MT->ticketNum;
		meal->tail += 1;  

		return 1;
	}
	
}


int dequeue(char *MTQ_ID, struct mealTicket *MT, struct MTQ *meal, int ticketNum){
	
	
	if(meal->tail == 1){
		printf("%s is empty\n", meal->name);
		return 0;
	}
	printf("ta num is %d, tail is %d\n", ticketNum, meal->tail);
	for(int i=0; i<meal->tail; i++){
		printf("taaaaa num is %d, tail is %d \n", meal->buffer[i].ticketNum, meal->tail);
		if(meal->buffer[i].ticketNum == ticketNum){
			printf("im here\n");
			meal->tail -= 1;
			for(int l=i ; l<meal->tail; l++){
				meal->buffer[l].ticketNum = meal->buffer[l+1].ticketNum;
				meal->buffer[l].dish = meal->buffer[l+1].dish;
			}
			return 1;
		}
	}

	printf("Ticket number not found %d\n", ticketNum);
	return 0;
	
	
}

void initMTQ(int length, struct MTQ meal, char MTQNAME[]){

	//printf("name is %s\n", MTQNAME);
	//strcpy(meal.name , MTQNAME);
	// 
	meal.name = MTQNAME;
	printf("name is %s\n", meal.name);
	meal.head = 0;
	meal.tail = 0;
	meal.length = length;

}

int main(){

	struct MTQ breakfast, lunch, dinner, bar;

	// initMTQ(4, breakfast, "Breakfast");
	// initMTQ(4, lunch, "Lunch");
	// initMTQ(4, dinner, "Dinner");
	// initMTQ(4, bar, "Bar");

	breakfast.name = "Breakfast";
	breakfast.head = 0;
	breakfast.tail = 1; 
	breakfast.length = 3;
	breakfast.buffer = malloc(breakfast.length * sizeof(struct mealTicket));

	lunch.name = "Lunch";
	lunch.head = 0;
	lunch.tail = 1; 
	lunch.length = 3;
	lunch.buffer = malloc(lunch.length * sizeof(struct mealTicket));

	dinner.name = "Dinner";
	dinner.head = 0;
	dinner.tail = 1; 
	dinner.length = 3;
	dinner.buffer = malloc(dinner.length * sizeof(struct mealTicket));

	bar.name = "Bar";
	bar.head = 0;
	bar.tail = 1; 
	bar.length = 3;
	bar.buffer = malloc(bar.length * sizeof(struct mealTicket));


	struct MTQ *registry[4];
	
	registry[0] = &breakfast;
	registry[1] = &lunch;
	registry[2] = &dinner;
	registry[3] = &bar;

	struct mealTicket mt1, mt2, mt3;
	mt1.ticketNum = 321;
	mt1.dish = "yolo";

	mt2.ticketNum = 123;
	mt2.dish = "bolo";

	mt3.ticketNum = 1223;
	mt3.dish = "bolooooo";

	enqueue("Breakfast", &mt1, registry[0] );
	enqueue("Breakfast", &mt2, registry[0] );
	enqueue("Breakfast", &mt3, registry[0] );

	dequeue("Breakfast", &mt1, registry[0], 123 );
//	dequeue("Breakfast", &mt1, registry[0], 1223 );
	for(int i=0; i<2; i++){
		printf("regi name %d, \n", registry[0]->buffer[i].ticketNum);
		printf("regi name %s, \n", registry[0]->buffer[i].dish);
		printf("regi name %d, \n", registry[0]->tail);
	}

	free(breakfast.buffer);
	free(lunch.buffer);
	free(dinner.buffer);
	free(bar.buffer);
	return 0;
}
