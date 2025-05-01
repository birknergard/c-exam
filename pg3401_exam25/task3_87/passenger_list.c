#include "passenger_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "debug.h"

/* TODO: store a bool with data indicating whether pData was malloced or not */
/* TODO: Implement type declaration for list with LIST.szType */

/*
 *	NODE is a generic struct which can hold a pointer to any data,
 *	as well as a pointer to the "next" node.
 * */
/* TODO: MAKE MORE LIKE FLIGHT_LIST */
static PASSENGER *_NewPassenger(PASSENGER_DATA pd){
	PASSENGER *ppNew; 

	ppNew = (PASSENGER *) malloc(sizeof(PASSENGER));

	ppNew->ppdData = &pd;
	ppNew->ppNext = NULL;

	return ppNew;
}

/*
 *	LIST struct which can hold a pointer to any data,
 *	as well as a pointer to the "next" node.
 * */
static int _DestroyPassenger(PASSENGER *pp){
	if(pp == NULL){
		puts("Cannot free null pointer.");	
		return 1;
	}

	free(pp->ppdData);
	free(pp);
	return 0;
}

static PASSENGER *_GetPassenger(PASSENGER_LIST *ppl, int n){
	int i;
	PASSENGER *ppCurrent = NULL;
	ppCurrent = ppl->ppFirst;	

	/* Runs n times */
	for(i = 0; i < n; i++){
		ppCurrent = ppCurrent->ppNext;
	}

	return ppCurrent;
}

PASSENGER_LIST *CreatePassengerList(){
	PASSENGER_LIST *pplNew;
	pplNew = (PASSENGER_LIST *) malloc(sizeof(PASSENGER_LIST));

	pplNew->iLength = 0;
	pplNew->ppFirst = NULL;

	return pplNew;
}


int DestroyPassengerList(PASSENGER_LIST **pppl){
	/* for tracking current node */
	if(*pppl == NULL)
	   	return 1;

	if((*pppl)->ppFirst == NULL){
		free(pppl);	
	}

	PASSENGER *ppCurrent, *ppNext;
	int i;

	ppCurrent = (*pppl)->ppFirst;

	while(ppCurrent != NULL){
		ppNext = ppCurrent->ppNext;
		_DestroyPassenger(ppCurrent);
		ppCurrent = ppNext;
	}

	free(pppl);
}

void AddPassenger(PASSENGER_LIST *ppl, PASSENGER_DATA pd){
	PASSENGER *ppNewPassenger;		
	ppNewPassenger = _NewPassenger(pd); 

	/* If list is empty, creates new node at head */
	if(ppl->iLength == 0){
		ppl->ppFirst = ppNewPassenger;	
		ppl->iLength++;
		return;
	}

	ppNewPassenger->ppNext = ppl->ppFirst;
	ppl->ppFirst = ppNewPassenger;
	ppl->iLength++;	
}


PASSENGER_DATA *GetPassengerData(PASSENGER_LIST *ppl, int n){
	return _GetPassenger(ppl, n)->ppdData;
}

/*
 * TODO: FIX
 * */
int RemovePassenger(PASSENGER_LIST *ppl, int iSeatNumber){
	PASSENGER *ppOldFirst = NULL;

	if(ppl->iLength == 0){
		printf("Cant remove element on empty list.\n");
		return 1;
	}

	return 0;
}

/* For internal testing 
int main(void){
	printf("DEBUG1: Declaring LIST\n");

	LL_SINGLE *liStart;
	int **ip, i, a;

	a = 25;

	liStart = NewList();
	if(liStart == NULL){
		puts("DEBUG: liStart is NULL");
		return 1;
	}
		
	
	ip = (int**) malloc(sizeof(long*) * 100);
	for(i = 0; i < 100; i++){
		ip[i] = (int*) malloc(sizeof(int));
		*ip[i] = i; 
	}

	for(i = 0; i < 100; i++){
		Push(liStart, ip[i]);	
	}


	printf("\n\n Current list (%d):", liStart->iLength);
	for(i = 0; i < liStart->iLength; i++){
		printf(" %d", *(int*) GetValue(liStart, i));	
	}
	printf("\n");


	printf("\n\n Popping first two values.");
	RemoveFirst(liStart);
	RemoveFirst(liStart);

	printf("\n Popping last three values.");
	RemoveLast(liStart);
	RemoveLast(liStart);
	RemoveLast(liStart);

	printf("\n\n Current list:");
	for(i = 0; i < liStart->iLength; i++){
		printf(" %d", *(int*) GetValue(liStart, i));	
	}
	printf("\n");

	printf("\n Appending 25 to end of list.");
	Append(liStart, &a);

	printf("\n\n Current list:");
	for(i = 0; i < liStart->iLength; i++){
		printf(" %d", *(int*) GetValue(liStart, i));	
	}
	printf("\n");

	FreeList(liStart);

	for(i = 0; i < 100; i++)
		free(ip[i]);	
	free(ip);

	return 0;
}
*/
