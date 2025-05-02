#include "passenger_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "debug.h"

/*
 *
 * */
static PASSENGER *_CreatePassenger(int iSeatNumber, char szName[], int iAge){
	/* Declare new passenger pointer */
	PASSENGER *ppNew = NULL; 

	bdebug("Creating passenger\n");
	/* Checks that chosen name isn't too long */
	if(strlen(szName) > MAX_NAME){
		printf("Chosen name is too long.\n");
		return NULL;
	}

	/* Allocate new passenger pointer */
	ppNew = (PASSENGER *) malloc(sizeof(PASSENGER));
	if(ppNew == NULL){
		berror("Failed malloc in _CreatePassenger()\n");
		return NULL;
	}

	/* Initialize it */
	memset(ppNew, 0, sizeof(PASSENGER));
	ppNew->ppdData = NULL;
	ppNew->ppNext = NULL;

	/* Allocate for passenger data to the heap. This allows for more consistent freeing behavior. */
	ppNew->ppdData = (PASSENGER_DATA *) malloc(sizeof(PASSENGER_DATA)); 
	if(ppNew->ppdData == NULL){
		berror("Failed malloc in _CreatePassenger(), for passenger data\n");
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
		puts("Cannot free null pointer.\n");	
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
		puts("Passenger list is empty.\n");
		return NULL;
	}

	iNewNameLength = strlen(szName);
	if(iNewNameLength > MAX_NAME){
		puts("Given name exceeds max character limit on names. Therefore it does not exist in list.\n");
		return NULL;
	}

	/* From util.c: Sets inputted name to lowercase before searching */
	StrncpyLowercase(szNewNameLower, szName, iNewNameLength);

	/* Start by checking the first passenger in the list */
	ppCurrent = ppl->ppFirst;	

	while(ppCurrent != NULL){

		/* Should be safe length since its already in the list */
		iCurrentNameLength = strlen(ppCurrent->ppdData->szName);
		StrncpyLowercase(szCurrentNameLower, ppCurrent->ppdData->szName, iCurrentNameLength);

		/* If the current passenger has the same name: return passenger */
		if(strcmp(szCurrentNameLower, szNewNameLower) == 0){
			return ppCurrent;
		}

		/* If not: reset name buffer check next */
		ppCurrent = ppCurrent->ppNext;
	}


	return NULL;
}

PASSENGER_DATA *GetPassengerData(PASSENGER_LIST *ppl, char szName[]){
	PASSENGER_DATA *ppdData = NULL;
	PASSENGER *ppPassenger = NULL;
	ppPassenger = _GetPassenger(ppl, szName);

	if(ppPassenger == NULL){
		berror("Passenger was not found.\n");
		return NULL;
	}

	ppdData = ppPassenger->ppdData;
	if(ppdData == NULL){
		berror("Passenger data was not found.\n");
		return NULL;
	}

	ppPassenger = NULL;
	return ppdData; 
}

PASSENGER_LIST *CreatePassengerList(){
	PASSENGER_LIST *pplNew = NULL;
	pplNew = (PASSENGER_LIST *) malloc(sizeof(PASSENGER_LIST));
	if(pplNew == NULL){
		berror("CreatePassengerList() -> Malloc failed");
		return NULL;
	}

	memset(pplNew, 0, sizeof(PASSENGER_LIST));

	pplNew->iLength = 0;
	pplNew->ppFirst = NULL;

	return pplNew;
}


int DestroyPassengerList(PASSENGER_LIST *pppl){
	/* Declaring variables */
	PASSENGER *ppCurrent = NULL, *ppNext = NULL;
	int i;

	/* Errors if attempting to destroy empty list */
	if(pppl == NULL){
		berror("Attempted to destroy an empty list.\n");
		return ERROR;
	}

   /* If ppCurrent ends up being NULL it passes the while check, freeing the node
      Otherwise it enters the while */
	ppCurrent = pppl->ppFirst;

	while(ppCurrent != NULL){
		ppNext = ppCurrent->ppNext;
		_DestroyPassenger(ppCurrent);
		ppCurrent = ppNext;
	}

	ppCurrent = NULL;
	ppNext = NULL;
	free(pppl);
	return OK;
}

/* Make sure seat numbers are unique and have a ceiling(Can plane be full?) and floor(0).
 * This list list is supposed to be sorted by SEAT NUMBER. 
 * TODO: Check tabs (too large)
 * */
int AddPassenger(PASSENGER_LIST *ppl, int iSeatNumber, char szName[], int iAge){
	PASSENGER *ppNewPassenger = NULL;		
	PASSENGER *ppCurrent = NULL;		
	PASSENGER *ppPrev = NULL;		
	int iCompareResult, iStatus;

	ppNewPassenger = _CreatePassenger(iSeatNumber, szName, iAge); 
	/* Checks if creation failed */
	if(ppNewPassenger == NULL){
		berror("Could not create add new passenger due to allocation error.\n");
		return ERROR;
	}

	/* Checks if passenger with that name already exists in the list */
	if(GetPassengerData(ppl, szName) != NULL){
		berror("Person already exists.");	
		return ERROR;
	}

	/* If list is empty, inserts passenger in first position */
	bdebug("Adding passenger to list.");
	if(ppl->iLength == 0 || ppl->ppFirst == NULL){
		ppl->ppFirst = ppNewPassenger;	
		ppl->iLength++;
		return OK;
	}

	/* Set current to first node */
	ppCurrent = ppl->ppFirst;
	ppPrev = ppCurrent;

	if (ppl->ppFirst->ppdData->iSeatNumber > iSeatNumber) {
		ppNewPassenger->ppNext = ppl->ppFirst;
		ppl->ppFirst = ppNewPassenger;
		ppl->iLength++;

		ppNewPassenger = NULL;		
		ppCurrent = NULL;		
		ppPrev = NULL;		
		return OK;
	}

	/* Effectively starting from the second node, until current reaches null pointer */
	while(ppCurrent != NULL){
		iCompareResult = ppCurrent->ppdData->iSeatNumber < iSeatNumber;

		/* This should be prevented before this function is invoked. This is here as a precaution. */
		if(ppCurrent->ppdData->iSeatNumber == iSeatNumber){
			berror("Duplicate seat number not allowed in passenger list.\n");
			free(ppNewPassenger);
			iStatus = ERROR;
			break;
		}

		// Key is greater
		if(iCompareResult > 0){
			// If higher value, go next, unless next passenger is NULL
			if(ppCurrent->ppNext == NULL){
				ppCurrent->ppNext = ppNewPassenger;
				ppl->iLength++;
				break;
			} 

			ppPrev = ppCurrent;
			ppCurrent = ppCurrent->ppNext;       

			// Key is smaller, element is insert before the next key 
		} else {
			// handle insert
			ppPrev->ppNext = ppNewPassenger;
			ppNewPassenger->ppNext = ppCurrent;
			ppl->iLength++;
			break;
		}
	}

	ppNewPassenger = NULL;		
	ppCurrent = NULL;		
	ppPrev = NULL;		
	return iStatus;
}

/*
 * 
 * */
int RemovePassenger(PASSENGER_LIST *ppl, char szName[]){
	PASSENGER *ppCurrent = NULL;
	PASSENGER *ppTarget = NULL;
	PASSENGER *ppPrev = NULL;
	int iStatus = ERROR;

	if(ppl->iLength == 0){
		printf("You can't remove from an empty list.\n");
		return ERROR;
	}

	ppCurrent = ppl->ppFirst;

	/* Checks head node first */
	if(strncmp(szName, ppCurrent->ppdData->szName, MAX_NAME) == 0){
		bdebug("Deleting %s\n", szName);
		ppTarget = ppCurrent;
		ppl->ppFirst = ppCurrent->ppNext;  // Move head to next node

		/* Delete passenger */
		_DestroyPassenger(ppTarget);
		ppl->iLength--;
		iStatus = OK;

	} else {
		/* Traverse the list for middle or last node */
		ppPrev = ppCurrent;
		ppCurrent = ppCurrent->ppNext;

		while(ppCurrent != NULL){
			if(strncmp(szName, ppCurrent->ppdData->szName, MAX_NAME) == 0){
				bdebug("Deleting %s\n", szName);
				ppTarget = ppCurrent;
				ppPrev->ppNext = ppCurrent->ppNext;  // Skip over the current node

				/* Delete passenger */
				_DestroyPassenger(ppTarget);
				ppl->iLength--;
				iStatus = OK;
				break;
			}

			ppPrev = ppCurrent;  // Move to the next node
			ppCurrent = ppCurrent->ppNext;
		}
	}

	if(ppTarget == NULL){
		printf("Could not find passenger named %s in list.\n", szName);
		return ERROR;
	}

	ppCurrent = NULL;
	ppTarget = NULL;
	ppPrev = NULL;
	return iStatus;

}

/* Assuming we handle the case of a duplicate seat number in the flight_list datastruct */
void ChangeSeat(PASSENGER_LIST *ppl, char szName[], int iNewSeat){
	PASSENGER_DATA *pdOriginalPassengerData = NULL;
	int iAge;

	pdOriginalPassengerData = GetPassengerData(ppl, szName);
	if(pdOriginalPassengerData != NULL){
		iAge = pdOriginalPassengerData->iAge;
		RemovePassenger(ppl, szName);
		AddPassenger(ppl, iNewSeat, szName, iAge);
	} else {
		printf("Passenger doesn't exist.\n");	
	}

	pdOriginalPassengerData = NULL;
}

void PrintPassengerList(PASSENGER_LIST *ppl){
	PASSENGER *ppCurrent = NULL;
	int n = 0;

	printf("   > PASSENGERS, %d people -\n", ppl->iLength);
	ppCurrent = ppl->ppFirst;

	while(ppCurrent != NULL){
		n++;
		printf("%d: %s, %d, %d\n",
	 n,
	 ppCurrent->ppdData->szName,
	 ppCurrent->ppdData->iSeatNumber,
	 ppCurrent->ppdData->iAge
	 );	
		ppCurrent = ppCurrent->ppNext;
	}
	puts("");

	ppCurrent = NULL;
}

void InternalPassengerListTest(){
	return;
}
/*
    PASSENGER_LIST *passengerList = CreatePassengerList();
    if (passengerList == NULL) {
	bdebug("Error: Failed to create passenger list.\n");
	return;
    }
    bdebug("Created a new passenger list.\n");

    // Add passengers with different seat numbers to test ordering
    bdebug("\nAdding passengers with varying seat numbers...\n");

    // Add passenger with seat number 2
    if (AddPassenger(passengerList, 2, "John Doe", 30) == OK) {
	bdebug("Passenger John Doe added (Seat 2).\n");
    }

    // Add passenger with seat number 5
    if (AddPassenger(passengerList, 5, "Jane Smith", 25) == OK) {
	bdebug("Passenger Jane Smith added (Seat 5).\n");
    }

    // Add passenger with seat number 1 (this should be added at the beginning)
    if (AddPassenger(passengerList, 1, "Alice Johnson", 35) == OK) {
	bdebug("Passenger Alice Johnson added (Seat 1).\n");
    }

    // Add passenger with seat number 3 (this should go between 2 and 5)
    if (AddPassenger(passengerList, 3, "Bob Lee", 28) == OK) {
	bdebug("Passenger Bob Lee added (Seat 3).\n");
    }

    // Add passenger with seat number 4 (this should go between 3 and 5)
    if (AddPassenger(passengerList, 4, "Charlie Brown", 40) == OK) {
	bdebug("Passenger Charlie Brown added (Seat 4).\n");
    }

    // Print the list of passengers to check the order
    bdebug("\nPassenger List (after adding passengers with varying seat numbers):\n");
    PrintList(passengerList);

    // Verify the list is sorted by seat number
    bdebug("\nVerifying seat number order:\n");
    PASSENGER *ppCurrent = passengerList->ppFirst;
    int previousSeatNumber = 0;
    int isOrdered = 1;
    while (ppCurrent != NULL) {
	if (ppCurrent->ppdData->iSeatNumber < previousSeatNumber) {
	    bdebug("Error: Seat numbers are not in order! Seat number %d follows seat number %d.\n",
		ppCurrent->ppdData->iSeatNumber, previousSeatNumber);
	    isOrdered = 0;
	    break;
	}
	previousSeatNumber = ppCurrent->ppdData->iSeatNumber;
	ppCurrent = ppCurrent->ppNext;
    }
    if (isOrdered) {
	bdebug("Seat numbers are correctly ordered.\n");
    }

    // Remove passengers and verify the list updates correctly
    bdebug("\nRemoving a few passengers...\n");
    RemovePassenger(passengerList, "John Doe");
    bdebug("Removed John Doe.\n");

    // Print the list after removal
    bdebug("\nPassenger List After Removing John Doe:\n");
    PrintList(passengerList);

    // Verify the seat numbers are still ordered after removal
    bdebug("\nVerifying seat number order after removal:\n");
    ppCurrent = passengerList->ppFirst;
    previousSeatNumber = 0;
    isOrdered = 1;
    while (ppCurrent != NULL) {
	if (ppCurrent->ppdData->iSeatNumber < previousSeatNumber) {
	    bdebug("Error: Seat numbers are not in order! Seat number %d follows seat number %d.\n",
		ppCurrent->ppdData->iSeatNumber, previousSeatNumber);
	    isOrdered = 0;
	    break;
	}
	previousSeatNumber = ppCurrent->ppdData->iSeatNumber;
	ppCurrent = ppCurrent->ppNext;
    }
    if (isOrdered) {
	bdebug("Seat numbers are still correctly ordered after removal.\n");
    }

    // Destroy the passenger list
    bdebug("\nDestroying the passenger list...\n");
    DestroyPassengerList(passengerList);
    bdebug("Passenger list destroyed successfully.\n");
	*/
