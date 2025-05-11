/* 
 * TITLE: Flight list
 * AUTHOR: 87
 * DESCRIPTION: Implementation of flight list as a combination of:
 *		Doubly linked list (Flight list)
 *		Sorted singly linked list (Passenger List)
 *		
 *		as a bonus, Dynamic array (one way) in UniquePassengers 
 * */
#include "flight_list.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"

/*
 * Internal method to compare PASSENGER structs.
 * Returns 0 (OK) if equal, 1 if not, -1 if error
 * */
static int _ComparePassengers(PASSENGER *pp1, PASSENGER *pp2){
	if(pp1 != NULL && pp2 != NULL){

		/* If the struct values are equal, return OK */
		if(strcmp(pp1->pszName, pp2->pszName) == 0
				&& pp1->iAge == pp2->iAge) return 0;

		return 1;
	}

	berror("Attempted to compare one or two null pointers.\n");
	return -1;
}


static PASSENGER *_CreatePassenger(char szName[], int iAge){
	PASSENGER *ppNew = NULL; 
	int iNameLength = strlen(szName); 

	/* Verify input */
	if(iNameLength > MAX_NAME){
		/*bdebug("Name exceeds maximum characters.\n"); */
		return NULL;
	}

	ppNew = (PASSENGER *) malloc(sizeof(PASSENGER)); 
	if(ppNew == NULL){
		berror("Failed malloc in _CreatePassenger(), for passenger data\n");
		free(ppNew);
		ppNew = NULL;
		return NULL;
	}

	/* Dynamically allocates name based on length. Since length is checked above this is safe */
	ppNew->pszName = (char *) malloc(iNameLength + 1);
	if(ppNew->pszName == NULL){
		free(ppNew);
		ppNew = NULL;
		return NULL;
	}
	/* Inputs data into fields */
	ppNew->iAge = iAge;

	/* Copies name string to newly allocated string */
	strncpy(ppNew->pszName, szName, iNameLength);
	ppNew->pszName[iNameLength] = '\0'; 

	return ppNew;
}

/*
 * This internal function creates a PASSENGER*, given a passengers seat number, name, and age.
 * */
static PASSENGER_NODE *_CreatePassengerNode(int iSeatNumber, PASSENGER *ppPassenger){
	/* Declare new passenger pointer */
	PASSENGER_NODE *ppnNew = NULL; 

	/* Allocate new passenger pointer */
	ppnNew = (PASSENGER_NODE *) malloc(sizeof(PASSENGER_NODE));
	if(ppnNew == NULL){
		berror("Failed malloc in _CreatePassenger()\n");
		return NULL;
	}

	/* Initialize it */
	ppnNew->ppPassenger = NULL;

	/* Defaults seat number to invalid number */
	ppnNew->iSeatNumber = iSeatNumber;
	ppnNew->ppnNext = NULL;

	/* Makes data field point to given passenger address */
	ppnNew->ppPassenger = ppPassenger; 
	if(ppnNew->ppPassenger == NULL){
		berror("Failed assign for passenger data\n");
		free(ppnNew);
		ppnNew = NULL;
		return NULL;
	}

	/* Return the new node pointer */
	return ppnNew;
}

/*
 * Simple function which destroys a passenger pointer
 * */
static int _DestroyPassenger(PASSENGER *pp){
	if(pp != NULL){
		free(pp->pszName);
		free(pp);
		return 0;
	}

	berror("Attempted to free null pointer.");
	return -1;
};

/*
 * This function deallocates/frees a given PASSENGER NODE* and all its corresponding data.
 * It returns 0 if successful, otherwise it returns 1.
 * */
static int _DestroyPassengerNode(PASSENGER_NODE *ppn){
	/* Verify pointer is allocated */
	if(ppn == NULL){
		berror("Cannot free null pointer.\n");	
		return -1;
	}

	/* Release connected node if exists, and destroy passenger data */
	ppn->ppnNext = NULL;

	/* Disconnects node from passenger data (ORIGINATES IN FLIGHTLIST STRUCT) */
	ppn->ppPassenger = NULL;

	/* Free/destroy the pointer */
	free(ppn);

	return 0;
}

/*
 * Retrieves a passenger node by passenger pointer. Passenger pointer is assigned later in the source file.
 * */
static PASSENGER_NODE *_GetPassengerNode(PASSENGER_LIST *ppl, PASSENGER *ppPassenger){
	PASSENGER_NODE *ppnCurrent = NULL;

	/* Verifies list is not empty */
	if(ppl->iLength == 0){
		//bdebug("Passenger list is empty.");
		return NULL;
	}

	/* Start by checking the first passenger in the list */
	ppnCurrent = ppl->ppnHead;	

	/* Checks list until hitting NULL pointer */
	while(ppnCurrent != NULL){

		/* If the current passenger has the same name we return the passenger */ 
		if(_ComparePassengers(ppnCurrent->ppPassenger, ppPassenger) == 0){
			return ppnCurrent;
		};

		/* If not we check the next one */
		ppnCurrent = ppnCurrent->ppnNext;
	}

	/* Cleanup */
	ppnCurrent = NULL;
	return NULL;
}


/*
 * Creates an empty but initialized passenger list.
 * */
static PASSENGER_LIST *_CreatePassengerList(){
	/* Declaring pointer */
	PASSENGER_LIST *pplNew = NULL;

	/* Allocating and verifying */
	pplNew = (PASSENGER_LIST *) malloc(sizeof(PASSENGER_LIST));
	if(pplNew == NULL){
		berror("CreatePassengerList() -> Malloc failed");
		return NULL;
	}

	/* Initializing */
	memset(pplNew, 0, sizeof(PASSENGER_LIST));
	pplNew->iLength = 0;
	pplNew->ppnHead = NULL;

	return pplNew;
}


/*
 * Destroys a previously created passenger list.
 * Passenger data is not destroyed here as it is handled in its 
 *  own data structure later (FLIGHT_LIST.pdUniquePassengers)
 * */
static int _DestroyPassengerList(PASSENGER_LIST *ppl){

	/* Declaring variables */
	PASSENGER_NODE *ppnCurrent = NULL, *ppnTemp = NULL;

	/* Errors if attempting to destroy empty list */
	if(ppl == NULL){
		berror("Attempted to destroy an empty list.\n");
		return -1;
	}

	/* If ppnCurrent ends up being NULL it passes the while check, freeing the node
	   Otherwise it enters the while loop */
	ppnCurrent = ppl->ppnHead;

	/* Destroys every node in list */
	while(ppnCurrent != NULL){
		ppnTemp = ppnCurrent;
		ppnCurrent = ppnCurrent->ppnNext;

		_DestroyPassengerNode(ppnTemp);
		/* makes sure its null before next loop */
		ppnTemp = NULL;
	}

	/* Cleanup */
	ppnCurrent = NULL;
	ppnTemp = NULL;
	free(ppl);
	return 0;
}

/* 
 * Checks (with a passenger) if a seat is taken by either that passenger or someone else
 * Returns 1 if true, 0 if not
 * */
static int _SeatTakenByPassenger(PASSENGER_LIST *ppl, int iSeatNumber, PASSENGER *ppPassenger){
	PASSENGER_NODE *ppnTemp = NULL;

	/* Checks if seat is available, returns 1 if not */
	ppnTemp = ppl->ppnHead;
	while(ppnTemp != NULL){

		/* Exit condition */
		if(ppnTemp->iSeatNumber == iSeatNumber){

			/* Seat is taken by ...(checks who occupies it)*/
			printf("Seat %d is ", iSeatNumber);

			/* .. someone else */
			if(_ComparePassengers(ppnTemp->ppPassenger, ppPassenger) == 1){
				printf(" taken by someone else!\n");
			}

			/* this passenger */
			else {
				printf(" already taken by %s.\n", ppPassenger->pszName);
			}

			ppnTemp = NULL; 
			return 1;
		}
		ppnTemp = ppnTemp->ppnNext;
	}

	ppnTemp = NULL;
	return 0;
}

/*
 * Adds a new passenger node in increasing order of seat number (1->MAX)
 * Returns 0 if successful, 1 if intentionally unsuccesful, and -1 if something went wrong.
 * */
static int _AddPassengerNode(PASSENGER_LIST *ppl, int iSeatNumber, PASSENGER *ppNew){
	PASSENGER_NODE *ppnNewNode = NULL;
	PASSENGER_NODE *ppnCurrent = NULL;
	PASSENGER_NODE *ppnPrev = NULL;
	int iSeatTaken = 0, iCompareResult;

	/* Checks if seat is taken by another (or the same) passenger */
	iSeatTaken = _SeatTakenByPassenger(ppl, iSeatNumber, ppNew);
	if(iSeatTaken == 1){
		return 1;
	}

	/* Checks if passenger already exists in the list */
	ppnNewNode = _GetPassengerNode(ppl, ppNew);
	if(ppnNewNode != NULL){
		printf("Passenger already exists in the list\n");
		return 1;
	}

	/* Allocates new passenger */
	ppnNewNode = _CreatePassengerNode(iSeatNumber, ppNew); 
	if(ppnNewNode == NULL){
		berror("Could not create add new passenger due to allocation error.\n");
		return -1;
	}

	/* If list is empty, inserts passenger in first position */
	if(ppl->iLength == 0 || ppl->ppnHead == NULL){
		ppl->ppnHead = ppnNewNode;	
		ppl->iLength++;
		ppnNewNode = NULL;
		return 0;
	}


	/* Set current to first node */
	ppnCurrent = ppl->ppnHead;
	ppnPrev = ppnCurrent;

	/* Checks if node can be inserted at head */
	if (ppl->ppnHead->iSeatNumber > iSeatNumber) {
		ppnNewNode->ppnNext = ppl->ppnHead;
		ppl->ppnHead = ppnNewNode;
		ppl->iLength++;

		ppnNewNode = NULL;		
		ppnCurrent = NULL;		
		ppnPrev = NULL;		
		return 0;
	}

	/* If not it continues until current reaches null pointer or finds correct adjacent node */
	while(ppnCurrent != NULL){
		iCompareResult = ppnCurrent->iSeatNumber < iSeatNumber;

		// Key is greater
		if(iCompareResult > 0){
			// If higher value, go next, unless next passenger is NULL
			if(ppnCurrent->ppnNext == NULL){
				ppnCurrent->ppnNext = ppnNewNode;

				/* Increments wrapper list */
				ppl->iLength++;
				break;
			} 

			/* Checks next node, stores current node */
			ppnPrev = ppnCurrent;
			ppnCurrent = ppnCurrent->ppnNext;       

			// Key is smaller, element is insert before the next key 
		} else {
			// handle insert
			ppnPrev->ppnNext = ppnNewNode;
			ppnNewNode->ppnNext = ppnCurrent;

			/* Increments wrapper list */
			ppl->iLength++;
			break;
		}
	}

	/* Cleanup */
	ppnNewNode = NULL;		
	ppnCurrent = NULL;		
	ppnPrev = NULL;		

	return 0;
}

/*
 * Removes a passenger from the list given a Passenger struct. 
 * */
static int _RemovePassenger(PASSENGER_LIST *ppl, PASSENGER *ppPassenger){
	PASSENGER_NODE *ppnCurrent = NULL;
	PASSENGER_NODE *ppnTarget = NULL;
	PASSENGER_NODE *ppnPrev = NULL;
	int iStatus = ERROR;

	if(ppl->iLength == 0){
		printf("You can't remove from an empty list.\n");
		return ERROR;
	}

	ppnCurrent = ppl->ppnHead;

	/* Checks head node first */
	if(_ComparePassengers(ppnCurrent->ppPassenger, ppPassenger) == OK){
		ppnTarget = ppnCurrent;
		ppl->ppnHead = ppnCurrent->ppnNext;  // Move head to next node

		/* Remove passenger reference */
		ppnCurrent->ppPassenger = NULL;

		/* Decrement list counter */  
		ppl->iLength--;
		iStatus = OK;

	} else {
		/* Else traverse until the last */
		ppnPrev = ppnCurrent;
		ppnCurrent = ppnCurrent->ppnNext;

		while(ppnCurrent != NULL){
			if(_ComparePassengers(ppnCurrent->ppPassenger, ppPassenger) == 0){
				ppnTarget = ppnCurrent;

				/* Omits the target */
				ppnPrev->ppnNext = ppnCurrent->ppnNext;

				/* Remove passenger reference */
				ppnTarget->ppPassenger = NULL;
				_DestroyPassengerNode(ppnTarget);

				/* Decrement list counter */  
				ppl->iLength--;
				iStatus = OK;
				break;
			}

			ppnPrev = ppnCurrent; 
			ppnCurrent = ppnCurrent->ppnNext;
		}
	}

	/* If target reaches null that means the node was not found */
	if(ppnTarget == NULL){
		printf("Could not find passenger named %s in list.\n", ppPassenger->pszName);
		return ERROR;
	}

	/* Cleanup */
	ppnCurrent = NULL;
	ppnTarget = NULL;
	ppnPrev = NULL;
	return iStatus;

}


/*
 * Changes the seat of a passenger, given a passenger pointer 
 * Essentially just removes the node and reinserts it again with the new seat number,
 *  allowing add/remove functions do most of the work, with some added input validation
 *  NOTE: Does check whether new seat is the same as the old one, or if the new one is
 *  taken by another passenger
 * */
static int _ChangeSeat(PASSENGER_LIST *ppl, PASSENGER *ppPassenger, int iNewSeat){
	/* Declares/initializes variables */
	PASSENGER_NODE *ppnPassengerNode = NULL;
	int iSeatTaken;

	/* Checks if seat is available, returns 1 if not */
	iSeatTaken = _SeatTakenByPassenger(ppl, iNewSeat, ppPassenger);
	if(iSeatTaken == 1){
		return 1;
	}

	/* Retrieves the pointer to the passengers node */
	ppnPassengerNode = _GetPassengerNode(ppl, ppPassenger);
	/* If passenger exists in list, the function executes */
	if(ppnPassengerNode != NULL){

		/* Stores the seat of the original node */
		ppnPassengerNode = NULL;

		/* Removes the passenger */
		if(_RemovePassenger(ppl, ppPassenger) == 1){
			return 1;
		} 

		/* Adds the passenger again (with new seat number) */
		if(_AddPassengerNode(ppl, iNewSeat, ppPassenger) == 1){
			return 1;
		} 

		/* If not, the function exits and prints and error. */
	} else {
		printf("Passenger doesn't exist.\n");	
		return 1;
	}

	/* Cleanup */
	ppnPassengerNode = NULL;
	return 0;
}

/*
 * Prints list of passengers
 * */
static void _PrintPassengerList(PASSENGER_LIST *ppl){
	/* Declaring variables */
	PASSENGER_NODE *ppnCurrent = NULL;
	int n = 0;

	/* Print passenger count */
	printf("   > PASSENGERS, %d people <\n", ppl->iLength);


	/* Print each passenger in list, along with their position in the list */
	ppnCurrent = ppl->ppnHead;
	while(ppnCurrent != NULL){
		n++;
		printf("   %d: %s, %d - SEAT:%d\n", n, 
				ppnCurrent->ppPassenger->pszName,
				ppnCurrent->ppPassenger->iAge,
				ppnCurrent->iSeatNumber
		      );	
		ppnCurrent = ppnCurrent->ppnNext;
	}
	puts("");

	/* Cleanup */
	ppnCurrent = NULL;
}

/* 
 * Converts all the characters in a string to uppercase 
 * */
static void ToUppercase(char *pszString[]){
	/* Sets lowercase letters to uppercase */
	int i;
	for(i = 0; i < 4; i++){
		if(123 > (*pszString)[i] && (*pszString)[i] > 96){
			(*pszString)[i] = (*pszString)[i] - 32;
		}
	}
}

/*
 * This function creates a new FLIGHT_NODE *, given its required data.
 * Also performs validation of arguments for added safety.
 * */
static FLIGHT_NODE *_CreateFlight(char szID[], int iDepartureTime, char szDestination[]){
	/* Declaring variables */
	FLIGHT_NODE *pfnCreated = NULL;
	int iDestinationSize = strlen(szDestination);
	int iIDSize = strlen(szID);

	/* Verifying string length for destination */
	if(iDestinationSize >= MAX_DESTINATION || iDestinationSize < 1){
		berror("Provided destination(%d) exceeds max character count for destinations (%d)", iDestinationSize, MAX_DESTINATION);
		return NULL;
	}

	/* Verifies ID size */
	if(iIDSize != 4){
		berror("Invalid ID (%d characters). Needs to be %d characters long\n", iIDSize,  MAX_ID - 1);
		return NULL;
	}


	/* Allocate new flight */
	pfnCreated = (FLIGHT_NODE *) malloc(sizeof(FLIGHT_NODE));
	if(pfnCreated == NULL){
		berror("Failed malloc in CreateFlight()");
		return NULL;
	}

	/* Initialize pointers to null */
	pfnCreated->pfnNext = NULL;
	pfnCreated->pfnPrev = NULL;

	/* Allocate data pointer */
	pfnCreated->pfdData = (FLIGHT_DATA *) malloc(sizeof(FLIGHT_DATA));
	if(pfnCreated->pfdData == NULL){
		berror("Failed malloc in CreateFlight() struct fdData\n");
		free(pfnCreated);
		pfnCreated = NULL;
		return pfnCreated;
	}
	/* Initializing destination pointer to null*/
	pfnCreated->pfdData->pszDestination = NULL;

	/* Initializing ID array*/
	memset(pfnCreated->pfdData->szID, 0, MAX_ID);
	pfnCreated->pfdData->pszDestination = (char *) malloc(iDestinationSize + 1);
	if(pfnCreated->pfdData->pszDestination == NULL){
		berror("Failed malloc for creating szDestination.\n");
		free(pfnCreated->pfdData);
		free(pfnCreated);
		return NULL;
	}

	/* Set data struct members */
	pfnCreated->pfdData->iDepartureTime = iDepartureTime;

	/* For string members, copying from input with buffer and null terminating them :) */
	strncpy(pfnCreated->pfdData->szID, szID, MAX_ID);
	pfnCreated->pfdData->szID[MAX_ID - 1] = '\0';

	strncpy(pfnCreated->pfdData->pszDestination, szDestination, iDestinationSize);
	pfnCreated->pfdData->pszDestination[iDestinationSize] = '\0';

	/* Creates the passenger list within the flight data */
	pfnCreated->pfdData->pplPassengers = _CreatePassengerList();
	if(pfnCreated->pfdData->pplPassengers == NULL){
		berror("Failed to create passenger list.");
		free(pfnCreated->pfdData->pszDestination);
		free(pfnCreated->pfdData);
		free(pfnCreated);
		pfnCreated = NULL;
	}

	/* If nothing went wrong, returns the new Flight List by address */
	return pfnCreated;
}

/*
 * Retrieves a FLIGHT_NODE * by its ID (4 letters), performs validation
 * */
static FLIGHT_NODE *_GetFlightByID(FLIGHT_LIST *pfl, char szID[]){
	FLIGHT_NODE *pfnCurrent = NULL;

	/* Checks if list is empty */
	if(pfl->pfnHead == NULL || pfl->pfnTail == NULL){
		berror("Can't search on an empty list.\n");
		return NULL;
	}

	/* Checks for valid ID input */
	if(strlen(szID) != 4){
		berror("Invalid flight id! Needs to be 4 digits/characters");
		return NULL;
	}

	/* Searches the list until it finds the right flight */
	pfnCurrent = pfl->pfnHead;
	while(pfnCurrent != NULL){
		if(strncmp(pfnCurrent->pfdData->szID, szID, MAX_ID) == 0){
			return pfnCurrent;
		}

		pfnCurrent = pfnCurrent->pfnNext;
	}

	return NULL;
}


/*
 * Destroys a FLIGHT_NODE * allocated by CreateFlight.
 * */
static int _DestroyFlight(FLIGHT_NODE *pfn){
	int iPassengerListDestroyed;

	if(pfn == NULL){
		berror("Flight list is NULL. Cannot destroy unintialized list.\n");
		return -1; 
	}

	iPassengerListDestroyed = _DestroyPassengerList(pfn->pfdData->pplPassengers);

	if(iPassengerListDestroyed == 1){
		berror("Could not destroy passenger list.\n");
		return -1;
	}

	free(pfn->pfdData->pszDestination);
	free(pfn->pfdData);
	free(pfn);

	return 0;
}

/*
 * Checks if FLIGHT ID is valid. returns 1 if false.
 * */
int isValidFlightID(FLIGHT_LIST *pfl, char szFlightID[]){

	FLIGHT_NODE *pfNew = NULL;

	if(strlen(szFlightID) != 4){
		printf("Invalid length. Needs to be 4 characters.");
		return -1;
	}

	/* Sets lowercase letters to uppercase */
	ToUppercase(&szFlightID);

	if(pfl->pfnHead == NULL){
		//bdebug("No flights registered\n");
		return 0;
	}

	/* Check if flight exists on that ID */
	pfNew = _GetFlightByID(pfl, szFlightID);
	if(pfNew != NULL){
		pfNew = NULL;
		return 1;
	} 

	return 0;
}

/*
 * Initializes the FLIGHT_LIST structure in the form of a unique pointer.
 * Internally it also creates a list of unique passenger pointers, which are what is added to 
 * the passenger lists later on.
 * */
FLIGHT_LIST *CreateFlightList(){
	/* Declaring pointer */
	FLIGHT_LIST *pflCreated = NULL;

	/* Allocating for flight list, and verifies */
	pflCreated = (FLIGHT_LIST *) malloc(sizeof(FLIGHT_LIST));
	if(pflCreated == NULL){
		bdebug("Failed malloc in CreateFlightList()");
		return NULL;
	}

	/* Allocates PASSENGER ** for holding passenger addresses. Will be increased in size 
	   dynamically as passengers are added. */
	pflCreated->arrppUniquePassengers = NULL;
	pflCreated->arrppUniquePassengers = (PASSENGER **) malloc(sizeof(PASSENGER *));
	if(pflCreated->arrppUniquePassengers == NULL){
		berror("Malloc failed.\n");
		free(pflCreated);
		pflCreated = NULL;
		return NULL; 
	}

	/* Initializing pointers and length trackers */
	pflCreated->iLength = 0;
	pflCreated->iUniquePassengers = 0;
	pflCreated->pfnHead = NULL;
	pflCreated->pfnTail = NULL;

	return pflCreated;
}

/*
 * Destroys a flight list created with CreateFlightList
 * */
int DestroyFlightList(FLIGHT_LIST *pfl){
	int i;
	/* Initialize pointers */
	FLIGHT_NODE *pfnCurrent = pfl->pfnHead;   
	FLIGHT_NODE *pfnTemp = NULL;

	/* Goes through the list, freeing each node and its corresponding data */
	while(pfnCurrent != NULL){
		pfnTemp = pfnCurrent;
		pfnCurrent = pfnCurrent->pfnNext;
		_DestroyFlight(pfnTemp);
	}

	/* Removes dangling pointers */
	pfnCurrent = NULL;
	pfnTemp = NULL;

	/* Frees list */
	pfl->pfnHead = NULL; 
	pfl->pfnTail = NULL; 

	/* If not empty, frees every passenger referenced in unique passenger array */
	if(pfl->arrppUniquePassengers[0] != NULL && pfl->iUniquePassengers > 0){
		for(i = 0; i < pfl->iUniquePassengers; i++){

			/* Invokes internal function to destroy passenger*/
			_DestroyPassenger(pfl->arrppUniquePassengers[i]);
		}
	}
	free(pfl->arrppUniquePassengers);
	free(pfl);

	return OK;
}

/*
 * Function for point four in the task description. Takes the FLIGHT_LIST, and a destination string.  
 * If any flights have that destination count of flights is returned. Otherwise it returns -1.
 * */
int PrintFlightsByDestination(FLIGHT_LIST *pfl, char szDestination[]){
	FLIGHT_NODE *pfnCurrent = NULL;
	int iPosition = 1; /* Starts at 1 */
	int iFoundMatch = 0;

	/* Checks if list if empty */
	if(pfl->pfnHead == NULL || pfl->pfnTail == NULL){
		berror("Can't search on an empty list.\n");

	}

	/* Verifies that input is within MAX_DESTINATION buffer */
	if(strlen(szDestination) >= MAX_DESTINATION){
		berror("Provided destination exceeds character limit (%d).\n", MAX_DESTINATION);
		return -1;
	}

	/* Starts at head */
	pfnCurrent = pfl->pfnHead;

	/* Searches the list until it reaches a null pointer (end) */
	while(pfnCurrent != NULL){

		/* Since we checked the character length of szDestination beforehand, strcmp is safe */
		if(strcmp(pfnCurrent->pfdData->pszDestination, szDestination) == 0){
			PrintFlight(pfl, iPosition);
			iFoundMatch++;
		}

		pfnCurrent = pfnCurrent->pfnNext;
		iPosition++;
	}


	pfnCurrent = NULL;
	/* Returns match count if more than 1, invalid value if nothing was found */
	if(iFoundMatch > 0)
		return iFoundMatch;
	else return -1;

}


/*
 * Gets a FLIGHT_NODE ** by its position in list the list (pfl). Starts at 1 (not zero-indexed) 
 * */
static FLIGHT_NODE *_GetFlightByPosition(FLIGHT_LIST *pfl, int n){
	/* Declaring variables */
	int i;
	FLIGHT_NODE *pfnCurrent = NULL;

	/* Checks if N is out of bounds for the list */
	if(n > pfl->iLength || n < 0){
		return NULL;
	}

	/* If N is 1, retrieve the HEAD of the list. */
	if(n == 1){
		if(pfl->pfnHead == NULL){
			berror("Flight list HEAD is not defined.\n");
			return NULL;
		}
		return pfl->pfnHead;
	}


	/* If N is the current lenght of the list, retrieve the TAIL of the list (the last node). */
	if(n == pfl->iLength){
		if(pfl->pfnTail == NULL){
			berror("Flight list TAIL is not defined.\n");
			return NULL;
		}
		return pfl->pfnTail; 
	}

	/* If index is smaller or equal to middle, Iterate forward from head ... 
NOTE: If number is odd the number is automatically rounded down to nearest whole number :) */
	if(n <= pfl->iLength / 2){
		pfnCurrent = pfl->pfnHead;
		for(i = 0; i < n; i++){
			pfnCurrent = pfnCurrent->pfnNext;
		}

		/* ... Else go backwards from tail */
	} else {
		pfnCurrent = pfl->pfnTail;
		for(i = 0; i < n; i++){
			pfnCurrent = pfnCurrent->pfnPrev;
		}
	} 

	return pfnCurrent;
}


/*
 * Checks if a passenger exists in the unique passenger list
 * Returns 1 if it exists, 0 if it does not.
 * */
static int _PassengerExists(FLIGHT_LIST *pfl, char szName[]){
	int i;

	/* Checking if passengerlist is empty */
	if(pfl->iUniquePassengers == 0){
		return 0;   
	}
	/* Checks every passenger in array for whether it exists or not */
	for(i = 0; i < pfl->iUniquePassengers; i++){
		if(strcmp(pfl->arrppUniquePassengers[i]->pszName, szName) == 0){
			return 1;
		}
	}
	return 0;
}

/*
 * This list handles creation of new PASSENGER structs.
 * It keeps them in a dynamically expanding array.
 * Does not delete if a passenger is removed from all flights. Had to draw the line somewhere :P
 * NOTE: Largely copied from the AddOption method in "menu.c" :)
 * */
int AddUniquePassenger(FLIGHT_LIST *pfl, char szName[], int iAge){
	/* Declare variables */
	PASSENGER *ppNewPassenger = NULL;
	PASSENGER **ppExtended = NULL;

	/* Checks if passenger exists. If yes then returns 1 */
	if(_PassengerExists(pfl, szName) == 1){
		return 1;
	}

	/* Create a new PASSENGER, returns -1 if fails */
	ppNewPassenger = _CreatePassenger(szName, iAge);
	if(ppNewPassenger == NULL){
		return -1;
	}

	/* If list is empty */
	if(pfl->iUniquePassengers == 0){

		/* Bitwise copy to already allocated address */
		pfl->arrppUniquePassengers[0] = ppNewPassenger;
		pfl->iUniquePassengers++;

		/* If menu contains previous elements */
	} else {
		/* Creating extended pointer */
		ppExtended = (PASSENGER **) malloc(sizeof(PASSENGER *) * (pfl->iUniquePassengers + 1));
		if(ppExtended == NULL){
			berror("Allocation to extended pointer failed.\n");
			return -1;
		}

		/* Copying old data to new ptr */
		memcpy(ppExtended, pfl->arrppUniquePassengers, sizeof(PASSENGER *) * (pfl->iUniquePassengers));

		/* Bitwise copy new data to new section of array address */
		ppExtended[pfl->iUniquePassengers] = ppNewPassenger;

		/* Incrementing passenger counter */
		pfl->iUniquePassengers++;

		/* Deleting old data */
		free(pfl->arrppUniquePassengers);

		/* Reassigning main ptr to new one with added data */
		pfl->arrppUniquePassengers = ppExtended;

		ppExtended = NULL;
		ppNewPassenger = NULL;
	}

	return 0;
}

/*
 * Retrieves a unique passenger from the arraylist. 
 * Returns NULL if it fails.
 * */
static PASSENGER *_GetUniquePassenger(FLIGHT_LIST *pfl, char szName[]){
	/* Searches until it finds a passenger with the same name */
	int i;

	/* If array is empty, return NULL */
	if(pfl->iUniquePassengers == 0){
		return NULL; 
	}

	/* else, search the list x times */
	for(i = 0; i < pfl->iUniquePassengers; i++){
		if(strcmp(pfl->arrppUniquePassengers[i]->pszName, szName) == OK){
			return pfl->arrppUniquePassengers[i];
		}
	}

	/* If it doesnt find it, return NULL */
	return NULL;
}


/*
 * Prints a list of passengers for a flight by its Flight ID.
 * */
int PrintPassengers(FLIGHT_LIST *pfl, char szFlightID[]){
	if(pfl->iLength == 0){
		printf("-> no flights have been added to the list\n");
		return -1;
	}

	FLIGHT_NODE *pfnFlight = NULL;

	pfnFlight = _GetFlightByID(pfl, szFlightID);
	if(pfnFlight == NULL){
		return -1;
	} 

	/* Defined in passenger_list.h */
	_PrintPassengerList(pfnFlight->pfdData->pplPassengers);
	return OK;
}

/*
 * Prints a single flights information by its position in the flight list (N)
 * */
int PrintFlight(FLIGHT_LIST *pfl, int n){

	FLIGHT_NODE *pfnFlight = NULL;

	/* Checks for list length */
	if(pfl->iLength == 0){
		printf("-> no flights have been added to the list\n");
		return 1;
	}

	/* Retrieves a flight by its position N, checks for errors */
	pfnFlight = _GetFlightByPosition(pfl, n);
	if(pfnFlight == NULL){
		return ERROR;
	}
	/* Prints the flight info */
	printf("%d -> ", n);
	printf("%s: TO:  %s, DEPARTS: %d", 
			pfnFlight->pfdData->szID,
			pfnFlight->pfdData->pszDestination,
			pfnFlight->pfdData->iDepartureTime
	      );

	/* Inserts a little pointer to the head and tail :) */
	if(pfnFlight == pfl->pfnHead && pfnFlight == pfl->pfnTail) printf("  <--HEAD/TAIL\n");
	else if(pfnFlight == pfl->pfnHead) printf("  <--HEAD\n");
	else if(pfnFlight == pfl->pfnTail) printf("  <--TAIL\n");
	else puts("");

	/* Prints number of available seats */
	printf("   Available seats: %d\n", (MAX_SEATS - pfnFlight->pfdData->pplPassengers->iLength));

	/* Prints the list of passengers for the flight */
	_PrintPassengerList(pfnFlight->pfdData->pplPassengers); /* See "passenger_list.h" for definition */

	return OK;
}

/*
 * Prints data for every flight in the list
 * */
void PrintFlightList(FLIGHT_LIST *pfl) {
	int n;

	if(pfl->iLength == 0){
		printf("-> no flights have been added to the list\n");
		return;
	}

	/* Runs the above function for every flight in the list */
	for(n = 1; n <= pfl->iLength; n++){
		if(PrintFlight(pfl, n) == ERROR){
			break;
		}
	}
	return;
}

/*
 * A version of PrintFlight that prints a less information.
 * */
static int _PrintFlightSimple(FLIGHT_NODE *pf){
	FLIGHT_DATA *pfdData = NULL;

	if(pf == NULL){
		return ERROR;
	}

	/* Stores data in temp pointer for better readability */
	pfdData = pf->pfdData;

	/* Print data */
	printf(
			"%s, %s, departs at %d",
			pfdData->szID, pfdData->pszDestination, pfdData->iDepartureTime
	      ); 
	printf(", %d passengers on flight\n", pfdData->pplPassengers->iLength);

	/* cleanup */
	pfdData = NULL;

	return OK;
}

/*
 * Same function as PrintFlightList except with less information per flight
 * */
int PrintFlightListSimple(FLIGHT_LIST *pfl){
	FLIGHT_NODE *pfnCurrent = NULL; 
	int n = 1;

	if(pfl->iLength == 0){
		printf("No flights available.\n\n");
		return 1;
	}

	/* Starts at HEAD, stores the flights to be printed */
	pfnCurrent = pfl->pfnHead;

	while(pfnCurrent != NULL){
		printf("%d) ", n);   
		_PrintFlightSimple(pfnCurrent);

		/* Checks next flight */
		n++;
		pfnCurrent = pfnCurrent->pfnNext;
	}

	/* cleanup */
	pfnCurrent = NULL;

	return 0;
}

/*
 * Returns whether a passenger record of a specific name exists
 * used when taking input from user
 * Returns 0 if true
 * */
int UniquePassengerExists(FLIGHT_LIST *pfl, char szPassengerName[]){
	int i, iNameLength;

	/* Sets length to variable */
	iNameLength = strlen(szPassengerName);

	/* Checks if new name exceeds input length */
	if(iNameLength > MAX_NAME || iNameLength == 0){
		printf("Your given name is too many characters. (MAX: %d)", MAX_NAME);
		return -1;
	}

	if(pfl->iUniquePassengers == 0){
		return 0;
	}

	/* Check every entry for the name */
	for(i = 0; i < pfl->iUniquePassengers; i++) {
		if(strcmp(pfl->arrppUniquePassengers[i]->pszName, szPassengerName) == 0){
			return 1;
		}
	}

	return 0;
}

/*
 *  Checks whether a flights passenger list is empty
 * */
int PassengerListIsEmpty(FLIGHT_LIST *pfl, char szFlightID[]){
	FLIGHT_NODE *pfnFlight = NULL;

	/* This function returns null when no match is found. There we can use it this way. */
	pfnFlight = _GetFlightByID(pfl, szFlightID);
	if(pfnFlight == NULL){
		printf("Flight does not exist on id %s", szFlightID);
		return 1;
	} 

	/* Checking both head and tracker for redundancy */
	if(pfnFlight->pfdData->pplPassengers->ppnHead == NULL && pfnFlight->pfdData->pplPassengers->iLength == 0){
		return 1;
	} 

	pfnFlight = NULL;
	return 0;
}

/*
 * 1. Inserts a new flight at the head of the list 
 * */
int AddFlight(FLIGHT_LIST *pfl, char szID[], char szDepartureTime[], char szDestination[]){
	/* Declaring variables */
	FLIGHT_NODE *pfNew = NULL;
	
	/* Checks if time is valid (HHMM) */
	if(strlen(szDepartureTime) != 4){
		printf("Invalid departure time.\n");
		return 1;
	}

	int i;
	for(i = 0; i < 4; i++){
		if(isdigit(szDepartureTime[i]) == 0){
			printf("Invalid departure time.\n");
			return 1;
		}
	}

	/* Sets lowercase letters to uppercase */
	ToUppercase(&szID);

	char szHours[3] = {0};
	for(i = 0; i < 2; i++){
		szHours[i] = szDepartureTime[i];  
	}
	szHours[2] = '\0';

	if(atoi(szHours) > 23){
		printf("Invalid departure time.\n");
		return 1;
	}

	if((szDepartureTime[2] - 48) > 5){
		printf("Invalid departure time.\n");
		return 1;
	}

	int iDepartureTime = atoi(szDepartureTime);

	/* Creates a new flight pointer */
	pfNew = _CreateFlight(szID, iDepartureTime, szDestination);

	/* If the pointer was created successfully, add it to the flight list */
	if (pfNew == NULL){
		berror("Error when creating flight.");
		return -1;
	}
	/* If head is undefined (list is empty), set new node as head and tail */
	if(pfl->pfnHead == NULL){
		pfl->pfnHead = pfNew;
		pfl->pfnTail = pfNew;

	} else {
		/*  Newnode next ptr to current head */
		pfNew->pfnNext = pfl->pfnHead;

		/*  set current head prev ptr to new node */
		pfl->pfnHead->pfnPrev = pfNew;

		/* Define new list head as new node */ 
		pfl->pfnHead = pfNew;
	}

	/* Increments the length of the list */
	pfl->iLength++;

	/* Cleanup */
	pfNew = NULL;
	return 0;
}

/*
 * 2. Adds a passenger to a flight (given its FlightId(sz)). 
 * Makes sure the passenger * is unique through the _GetUniquePassenger function.
 * */
int AddPassengerToFlight(FLIGHT_LIST *pfl, char szFlightID[], int iSeatNumber, char szName[]){
	/* Declaring pointers */
	FLIGHT_NODE *pfnFlight = NULL;
	PASSENGER *ppNewPassenger = NULL;
	int iAddedPassenger;

	ToUppercase(&szFlightID);

	/* Retrieves a flight by its ID */
	pfnFlight = _GetFlightByID(pfl, szFlightID);
	if(pfnFlight == NULL){
		printf("No flight exists on that ID.\n");
	}

	/* Checks seat is within bounds */
	if(iSeatNumber > MAX_SEATS || iSeatNumber < 0){
		printf("%d is not a valid seat number. Needs to be a number between 0 and %d\n", iSeatNumber, MAX_SEATS);
		return 1;
	}

	/* Attempts to add passenger to list */
	ppNewPassenger = _GetUniquePassenger(pfl, szName);
	if(ppNewPassenger == NULL){
		/*bdebug("Passenger was not added in unique passenger list before adding to flight\n"); */
		return -1;
	}

	/* This function handles to check if seat number is taken etc ... */
	iAddedPassenger = _AddPassengerNode(pfnFlight->pfdData->pplPassengers, iSeatNumber, ppNewPassenger);
	if(iAddedPassenger == 0){
		printf("%s was added to flight!\n", szName);
	} 

	/* Cleanup */
	ppNewPassenger = NULL;
	pfnFlight = NULL;

	return iAddedPassenger;
}

/*
 * 5. Removes and deletes a flight from the flight list
 * */
int RemoveFlight(FLIGHT_LIST *pfl, char szID[]){
	FLIGHT_NODE *pfTarget = NULL;
	FLIGHT_NODE *pfAdjacentLeft = NULL;
	FLIGHT_NODE *pfAdjacentRight = NULL;

	/* Checks list length */
	if(pfl->iLength == 0 || pfl->pfnHead == NULL){
		printf("You can't remove from an empty list.\n");
		return ERROR;
	}

	/* Sets lowercase letters to uppercase */
	ToUppercase(&szID);

	/* Verifies existence of flight ID */
	if((pfTarget = _GetFlightByID(pfl, szID)) == NULL){
		printf("Flight with ID -> %s does not exist.\n", szID); 
		return ERROR;
	}

	/* If target for deletion is the HEAD, set head to next node over */
	if(pfTarget == pfl->pfnHead){
		pfl->pfnHead = pfl->pfnHead->pfnNext;   
	}

	/* If target for deletion is the TAIL, set the tail to the node previous to the target */
	if(pfTarget == pfl->pfnTail){
		pfl->pfnTail = pfl->pfnTail->pfnPrev;   
	}

	/* Finds the adjacent nodes (even if they are null) */
	pfAdjacentLeft = pfTarget->pfnNext;
	pfAdjacentRight = pfTarget->pfnPrev;

	/* points the adjacent nodes to each other if they exist, omitting the target */
	if (pfAdjacentRight != NULL)
		pfAdjacentRight->pfnNext = pfAdjacentLeft;
	if (pfAdjacentLeft != NULL)
		pfAdjacentLeft->pfnPrev = pfAdjacentRight;

	/* Detach target */
	pfTarget->pfnNext = NULL;
	pfTarget->pfnPrev = NULL;

	/* Destroys the Flight node (See function definition) */
	_DestroyFlight(pfTarget);

	/* Decrements the list counter */
	pfl->iLength--;

	/* Cleanup */
	pfTarget = NULL;
	pfAdjacentLeft = NULL;
	pfAdjacentRight = NULL;

	return OK;
}

/*
 * 6. Changes the seat of a passenger, Function is really similar to the previous one so 
 * I mainly comment on the differences.
 * */
int ChangePassengerSeat(FLIGHT_LIST *pfl, char szFlightID[], char szName[], int iNewSeat){
	PASSENGER *ppPassenger = NULL;
	FLIGHT_NODE *pfnFlight = NULL;
	int iSeatChanged = -1;

	/* Sets lowercase letters to uppercase */
	ToUppercase(&szFlightID);

	pfnFlight = _GetFlightByID(pfl, szFlightID);
	if(pfnFlight == NULL){
		printf("No flight exists on that ID.\n");
		return 1;
	}

	if(iNewSeat > MAX_SEATS || iNewSeat < 0){
		printf("%d is not a valid seat number. Needs to be a number between 0 and %d\n", iNewSeat, MAX_SEATS);
		return 1;
	}

	ppPassenger = _GetUniquePassenger(pfl, szName);
	if(ppPassenger == NULL){
		printf("Could not find any passenger name %s.\n", szName);
		return 1;
	}
	/* NOTE: We did not add a new passenger like we did in the last function*/

	/* Attempt to change the seat of the passenger with the given name */
	iSeatChanged = _ChangeSeat(pfnFlight->pfdData->pplPassengers, ppPassenger, iNewSeat);

	ppPassenger = NULL;
	pfnFlight = NULL;

	return iSeatChanged;
}
/*
 * 7. Gets a list of every flight a passenger is booked to, returns the number of flights
 * */
int GetPassengersFlights(FLIGHT_LIST *pfl, char szName[], int iPrint){
	/* Declare variables */
	FLIGHT_NODE *pfnCurrentFlight = NULL;
	PASSENGER *ppPassenger = NULL;
	int iFlightsFound = 0;


	/* Checks if passenger exists */
	if(UniquePassengerExists(pfl, szName) != 1){
		/* Since the passenger has no flights, we return the initialized value (0) */
		return iFlightsFound;
	};

	ppPassenger = _GetUniquePassenger(pfl, szName);

	if(iPrint == 1) printf("Displaying lists of flights for %s:\n", szName);

	/* Traverse the whole flight list, printing every flight that the passenger is a part of */
	pfnCurrentFlight = pfl->pfnHead;
	while(pfnCurrentFlight != NULL){

		/* Traverse the passenger list, printing when finding match to input name */
		if(_GetPassengerNode(pfnCurrentFlight->pfdData->pplPassengers, ppPassenger) != NULL){
			iFlightsFound++;
			if(iFlightsFound > 0 && iPrint == 1){
				_PrintFlightSimple(pfnCurrentFlight);
			}
		}
		pfnCurrentFlight = pfnCurrentFlight->pfnNext;
	}

	/* Cleanup */
	pfnCurrentFlight = NULL;
	ppPassenger = NULL;

	return iFlightsFound;
}

/*
 * 8. Checks every flight and prints every passenger that is booked to more than one flight.
 * Returns 1 if no valid passengers are found, or 0 if someone is found.
 * */
int PrintPassengersWithMultipleFlights(FLIGHT_LIST *pfl){
	/* Declaring variables and initializing pointers */
	int i, iNumberOfFlights, iPersonPrinted;

	/* If these two trackers are low enough there is no point in executing */
	if(pfl->iUniquePassengers == 0){
		printf("No passengers added.");
		return 1;
	}

	if(pfl->iLength <= 1){
		printf("Not enough flights added.");
		return 1;
	}

	iPersonPrinted = 0;
	/* Checks for every unique passenger */
	for(i = 0; i < pfl->iUniquePassengers; i++){
		/* Initialize to 0, will person is on flight if match is found */
		iNumberOfFlights = GetPassengersFlights(pfl, pfl->arrppUniquePassengers[i]->pszName, 0);

		/* If a person was has more than 1 flight we print, and edit the return value to indicate this */
		if(iNumberOfFlights > 1){
			printf("-> %s is booked to %d flights!\n", pfl->arrppUniquePassengers[i]->pszName, iNumberOfFlights);
			iPersonPrinted = 1;
		}
	}

	if(iPersonPrinted > 0) return 0;
	else return 1;
}
