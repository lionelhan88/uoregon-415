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
	 	ticNum += 1;
	 	if(ticNum == 4){
	 		ticNum = 0;
	 		return 0;
	 	}
	}
	if(MTQ_ID != meal->name){
		return 0;
	}else{
		meal->buffer[ticNum].dish = MT->dish;
		meal->buffer[ticNum].ticketNum = MT->ticketNum;
		meal->tail += 1;
		return 1;
	}

}


int dequeue(char *MTQ_ID, struct mealTicket *MT, struct MTQ *meal){

	if(meal->tail == 1){
		return 0;
	}
	for(int i=0; i<meal->tail; i++){
		if(meal->buffer[i].ticketNum == MT->ticketNum){
			printf("Queue %s - Ticket Number: %d - Dish: %s \n", meal->name, meal->buffer[i].ticketNum, meal->buffer[i].dish);
			meal->tail -= 1;
	 		for(int l=i ; l<meal->tail; l++){
				meal->buffer[l].ticketNum = meal->buffer[l+1].ticketNum;
				meal->buffer[l].dish = meal->buffer[l+1].dish;
			}
			return 1;
		}
	}
	return 0;
}


int main(){

	struct MTQ breakfast, lunch, dinner, bar;

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

	struct mealTicket mt1, mt2, mt3, mt4, mt5, mt6, mt7, mt8, mt9, mt10, mt11, mt12;
	mt1.ticketNum = 1;
	mt1.dish = "bread";

	mt2.ticketNum = 2;
	mt2.dish = "juice";

	mt3.ticketNum = 3;
	mt3.dish = "toast";

	mt4.ticketNum = 4;
	mt4.dish = "rice";

	mt5.ticketNum = 5;
	mt5.dish = "noodle";

	mt6.ticketNum = 6;
	mt6.dish = "steak";

	mt7.ticketNum = 7;
	mt7.dish = "turkey";

	mt8.ticketNum = 8;
	mt8.dish = "soup";

	mt9.ticketNum = 9;
	mt9.dish = "lamb";

	mt10.ticketNum = 10;
	mt10.dish = "gin";

	mt11.ticketNum = 11;
	mt11.dish = "vodka";

	mt12.ticketNum = 12;
	mt12.dish = "whisky";

	int result;

	result = dequeue("Breakfast", &mt1, registry[0] );
	if(result == 0 && registry[0]->tail == 1){
		printf("The queue is empty\n");
		printf("Test Case A - Result Fail\n");
	}else{
		printf("Test Case A - Result Success\n");
	}


	result = enqueue("Breakfast", &mt1, registry[0] );
	if(result == 1){
	//	printf("Adding meal %s to Breakfast\n", mt1.dish);
		printf("Test Case D - Result Success\n");
	}else{
		printf("Test Case D - Result Fail\n");
	}
	enqueue("Breakfast", &mt2, registry[0] );
	enqueue("Breakfast", &mt3, registry[0] );

	enqueue("Lunch", &mt4, registry[1] );
	enqueue("Lunch", &mt5, registry[1] );
	enqueue("Lunch", &mt6, registry[1] );

	enqueue("Dinner", &mt7, registry[2] );
	enqueue("Dinner", &mt8, registry[2] );
	enqueue("Dinner", &mt9, registry[2] );

	enqueue("Bar", &mt10, registry[3] );
	enqueue("Bar", &mt11, registry[3] );
	enqueue("Bar", &mt12, registry[3] );

	result = enqueue("Breakfast", &mt1, registry[0] );
	if(result == 0 && registry[0]->tail == 4){
		printf("The queue is full\n");
		printf("Test Case C - Result Fail\n");
	}

//	dequeue("Breakfast", &mt_(0+1), registry[0] );

//	int result;
//
//	for(int i=0; i<4; i++){
//		dequeue("Breakfast", &mt(i+1), registry[0] );
//		dequeue("Lunch", &mt(i+4), registry[1] );
//		dequeue("Dinner", &mt(i+7), registry[2] );
//		dequeue("Bar", &mt(i+9), registry[3]);
//	}

	free(breakfast.buffer);
	free(lunch.buffer);
	free(dinner.buffer);
	free(bar.buffer);
	return 0;
}
