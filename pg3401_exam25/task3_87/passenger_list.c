#include "passenger_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "debug.h"

/*
 *
 * */
static PASSENGER *_CreatePassenger(int iSeatNumber, char szName[], int iAge){
	/* Declare new passenger pointer */
	PASSENGER *ppNew = NULL; 

	/* Checks that chosen name isn't too long */
	if(strlen(szName) > MAX_NAME){
		printf("Chosen name is too long.\n");
		return NULL;
	}

	/* Allocate new passenger pointer */
	ppNew = (PASSENGER *) malloc(sizeof(PASSENGER));
	if(ppNew == NULL){
		berror("Failed malloc in _CreatePassenger()");
		return NULL;
	}

	/* Initialize it */
	memset(ppNew, 0, sizeof(PASSENGER));
	ppNew->ppdData = NULL;
	ppNew->ppNext = NULL;

	/* Allocate for passenger data to the heap. This allows for more consistent freeing behavior. */
	ppNew->ppdData = (PASSENGER_DATA *) malloc(sizeof(PASSENGER_DATA)); 
	if(ppNew->ppdData == NULL){
		berror("Failed malloc in _CreatePassenger(), for passenger data");
		free(ppNew);
		return NULL;
	}

	ppNew->ppdData->iSeatNumber = iSeatNumber;
	strncpy(ppNew->ppdData->szName, szName, strlen(szName));
	ppNew->ppdData->szName[strlen(szName)] = '\0';
	ppNew->ppdData->iAge = iAge;

	/* Return the new passenger pointer */
	return ppNew;
}

/*
 *
 * */
static int _DestroyPassenger(PASSENGER *pp){
	/* Verify pointer is allocated */
	if(pp == NULL){
		puts("Cannot free null pointer.");	
		return ERROR;
	}

	/* Release connected node if exists, and destroy passenger data */
	pp->ppNext = NULL;
	free(pp->ppdData);

	/* Free/destroy the pointer */
	free(pp);

	return OK;
}

static PASSENGER *_GetPassenger(PASSENGER_LIST *ppl, char szName[]){
	PASSENGER *ppCurrent = NULL;
	char szNewNameLower[MAX_NAME], szCurrentNameLower[MAX_NAME];
	int i, iNewNameLength, iCurrentNameLength;
	
	/* Verifies list is not empty */
	if(ppl->iLength == 0){
		puts("Passenger list is empty.");
		return NULL;
	}

	iNewNameLength = strlen(szName);
	if(iNewNameLength > MAX_NAME){
		puts("Given name exceeds max character limit on names. Therefore it does not exist in list.");
		return NULL;
	}

	/* From util.c: Sets name to lowercase before searching */
	StrncpyLowercase(szNewNameLower, szName, iNewNameLength);

	ppCurrent = ppl->ppFirst;	
	while(ppCurrent != NULL){

		/* Should be safe length since its already in the list */
		iCurrentNameLength = strlen(ppCurrent->ppdData->szName);
		StrncpyLowercase(szCurrentNameLower, ppCurrent->ppdData->szName, iCurrentNameLength);

		/* If the current passenger has the same name: return passenger */
		if(strncmp(szCurrentNameLower, szNewNameLower, MAX_NAME) == 0){
			return ppCurrent;
		}

		/* If not: reset name buffer check next */
		memset(szCurrentNameLower, 0, strlen(szName));
		ppCurrent = ppCurrent->ppNext;
	}

	return NULL;
}

PASSENGER_DATA *GetPassengerData(PASSENGER_LIST *ppl, char szName[]){
	return _GetPassenger(ppl, szName)->ppdData;
}

PASSENGER_LIST *CreatePassengerList(){
	PASSENGER_LIST *pplNew;
	pplNew = (PASSENGER_LIST *) malloc(sizeof(PASSENGER_LIST));
	memset(pplNew, 0, sizeof(PASSENGER_LIST));

	pplNew->iLength = 0;
	pplNew->ppFirst = NULL;

	return pplNew;
}


int DestroyPassengerList(PASSENGER_LIST **pppl){
	/* Declaring variables */
	PASSENGER *ppCurrent = NULL, *ppNext = NULL;
	int i;

	/* Errors if attempting to destroy empty list */
	if(*pppl == NULL){
		berror("Attempted to destroy an empty list.");
	   	return ERROR;
	}

	/* If list is empty but still exists, free the list immediately */
	if((*pppl)->ppFirst != NULL){
		ppCurrent = (*pppl)->ppFirst;

		while(ppCurrent != NULL){
			ppNext = ppCurrent->ppNext;
			_DestroyPassenger(ppCurrent);
			ppCurrent = ppNext;
		}
	}

	free(pppl);
}

/* Make sure seat numbers are unique and have a ceiling(Can plane be full?) and floor(0).
 * This list list is supposed to be sorted by SEAT NUMBER. 
 * */
int AddPassenger(PASSENGER_LIST *ppl, int iSeatNumber, char szName[], int iAge){
	PASSENGER *ppNewPassenger = NULL;		
	PASSENGER *ppCurrent = NULL;		
	PASSENGER *ppPrev = NULL;		
	ppNewPassenger = _CreatePassenger(iSeatNumber, szName, iAge); 

	/* Checks if creation failed */
	if(ppNewPassenger == NULL){
		berror("Could not create add new passenger due to allocation error.");
		return ERROR;
	}

	/* If list is empty, inserts passenger in first position */
	if(ppl->iLength == 0){
		ppl->ppFirst = ppNewPassenger;	
		ppl->iLength++;
		return OK;
	}

	ppCurrent = ppl->ppFirst;

	while(ppCurrent != NULL){
		ppPrev = ppCurrent;
		ppCurrent = ppCurrent->ppNext;

		/* This should be prevented before this function is invoked. This is here as a precaution. */
		if(ppCurrent->ppdData->iSeatNumber == ppNewPassenger->ppdData->iSeatNumber){
			berror("Duplicates not allowed in passenger list.");
			if(ppPrev != NULL) ppPrev = NULL;
			if(ppCurrent != NULL) ppCurrent = NULL;
			return ERROR;
		}

		if(ppCurrent->ppdData->iSeatNumber > ppNewPassenger->ppdData->iSeatNumber){
			ppPrev->ppNext = ppNewPassenger;
			ppNewPassenger->ppNext = ppCurrent;

			ppCurrent = NULL;
			ppPrev = NULL;

			return OK;
		}
	}

	ppPrev = NULL;
	ppNewPassenger = NULL;

	printf("Could not find an empty seat for %s.", szName);
	return ERROR;
}



/*
 * TODO: FIX
 * */
int RemovePassenger(PASSENGER_LIST *ppl, int iSeatNumber){
	PASSENGER *ppDeleted = NULL;

	if(ppl->iLength == 0){
		printf("Cant remove passenger on an empty list.\n");
		return ERROR;
	}

	return OK;
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
