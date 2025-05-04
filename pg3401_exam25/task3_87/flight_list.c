/* 
 * TITLE: Flight list
 * AUTHOR: 87
 * DESCRIPTION:
 * */

#include "flight_list.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "debug.h"

/*
 * Internal method to compare PASSENGER structs.
 * Returns 0 (OK) if equal, 1 if not, -1 if error
 * */
static int _ComparePassengers(PASSENGER *pp1, PASSENGER *pp2){
   if(pp1 != NULL && pp2 != NULL){
   
      /* If the struct values are equal, return OK */
      if(strcmp(pp1->pszName, pp2->pszName) == 0
      && pp1->iAge == pp2->iAge) return OK;
      
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
      bdebug("Name exceeds maximum characters.\n");
      return NULL;
   }

	ppNew = (PASSENGER *) malloc(sizeof(PASSENGER)); 
	if(ppNew == NULL){
		berror("Failed malloc in _CreatePassenger(), for passenger data\n");
		free(ppNew);
      ppNew = NULL;
		return NULL;
	}

   /* Dynamically allocates name based on length. Since length is check above this is safe */
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
	//memset(ppnNew, 0, sizeof(PASSENGER_NODE));
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
      return OK;
   }

   berror("Attempted to free null pointer.");
   return ERROR;
};

/*
 * This function deallocates/frees a given PASSENGER NODE* and all its corresponding data.
 * It returns 0 if successful, otherwise it returns 1.
 * */
static int _DestroyPassengerNode(PASSENGER_NODE *ppn){
	/* Verify pointer is allocated */
	if(ppn == NULL){
		berror("Cannot free null pointer.\n");	
		return ERROR;
	}

	/* Release connected node if exists, and destroy passenger data */
	ppn->ppnNext = NULL;

   /* Disconnects node from passenger data (ORIGINATES IN FLIGHTLIST STRUCT) */
   ppn->ppPassenger = NULL;

	/* Free/destroy the pointer */
	free(ppn);

	return OK;
}

/*
 * Retrieves a passenger node by passenger pointer. Passenger pointer is assigned later in the source file.
 * */
static PASSENGER_NODE *_GetPassengerNode(PASSENGER_LIST *ppl, PASSENGER *ppPassenger){
	PASSENGER_NODE *ppnCurrent = NULL;

	/* Verifies list is not empty */
	if(ppl->iLength == 0){
		bdebug("Passenger list is empty.");
		return NULL;
	}

	/* Start by checking the first passenger in the list */
	ppnCurrent = ppl->ppnHead;	

   /* Checks list until hitting NULL pointer */
	while(ppnCurrent != NULL){

		/* If the current passenger has the same name we return the passenger */ 
		if(_ComparePassengers(ppnCurrent->ppPassenger, ppPassenger) == OK){
			bdebug("Passenger found!\n");
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
		return ERROR;
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
	return OK;
}

/*
 * Adds a new passenger node in increasing order of seat number (1->MAX)
 * Returns 0 if successful, 1 if intentionally unsuccesful, and -1 if something went wrong.
 * */
static int _AddPassengerNode(PASSENGER_LIST *ppl, int iSeatNumber, PASSENGER *pp){
	PASSENGER_NODE *ppnNewPassenger = NULL;		
	PASSENGER_NODE *ppnCurrent = NULL;		
	PASSENGER_NODE *ppnPrev = NULL;		
	int iCompareResult, iStatus;


   ppnNewPassenger = _GetPassengerNode(ppl, pp);
	/* Checks if passenger with that name already exists in the list */
	if(ppnNewPassenger != NULL){
      if(ppnNewPassenger->iSeatNumber == iSeatNumber){
         printf("Passenger already has that seat number");
         return 1;
      };
	} else {
		printf("Person already exists in list.\n");	
		return 1;
   }



	/* Allocates new passenger */
	ppnNewPassenger = _CreatePassengerNode(iSeatNumber, pp); 
	if(ppnNewPassenger == NULL){
		berror("Could not create add new passenger due to allocation error.\n");
		return -1;
	}

	/* If list is empty, inserts passenger in first position */
	if(ppl->iLength == 0 || ppl->ppnHead == NULL){
		ppl->ppnHead = ppnNewPassenger;	
		ppl->iLength++;
		ppnNewPassenger = NULL;
		return 0;
	}

	/* Set current to first node */
	ppnCurrent = ppl->ppnHead;
	ppnPrev = ppnCurrent;

   /* Checks if node can be inserted at head */
	if (ppl->ppnHead->iSeatNumber > iSeatNumber) {
		ppnNewPassenger->ppnNext = ppl->ppnHead;
		ppl->ppnHead = ppnNewPassenger;
		ppl->iLength++;

		ppnNewPassenger = NULL;		
		ppnCurrent = NULL;		
		ppnPrev = NULL;		
		return 0;
	}

	/* If not it continues until current reaches null pointer or finds correct adjacent node */
	while(ppnCurrent != NULL){
		iCompareResult = ppnCurrent->iSeatNumber < iSeatNumber;

		/* This should be prevented before this function is invoked. This is here as a precaution. */
		/*if(ppnCurrent->iSeatNumber == iSeatNumber){
			printf("Seat number %d is taken.\n", iSeatNumber);
			//_DestroyPassengerNode(ppnNewPassenger);
			iStatus = 1;
			break;
		}*/

		// Key is greater
		if(iCompareResult > 0){
			// If higher value, go next, unless next passenger is NULL
			if(ppnCurrent->ppnNext == NULL){
				ppnCurrent->ppnNext = ppnNewPassenger;

            /* Increments wrapper list */
				ppl->iLength++;
				iStatus = OK;
				break;
			} 

         /* Checks next node, stores current node */
			ppnPrev = ppnCurrent;
			ppnCurrent = ppnCurrent->ppnNext;       

			// Key is smaller, element is insert before the next key 
		} else {
			// handle insert
			ppnPrev->ppnNext = ppnNewPassenger;
			ppnNewPassenger->ppnNext = ppnCurrent;

         /* Increments wrapper list */
			ppl->iLength++;

		   iStatus = OK;
			break;
		}
	}

   /* Cleanup */
	ppnNewPassenger = NULL;		
	ppnCurrent = NULL;		
	ppnPrev = NULL;		
	return iStatus;
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
				ppnPrev->ppnNext = ppnCurrent->ppnNext;

            /* Remove passenger reference */
            ppnCurrent->ppPassenger = NULL;

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
 * */
static int _ChangeSeat(PASSENGER_LIST *ppl, PASSENGER *ppPassenger, int iNewSeat){
   /* Declares/initializes variables */
	PASSENGER_NODE *ppnPassengerNode = NULL;
	int iOriginalSeat, iStatus = OK;

   /* Retrieves a pointer to the passengers node */
	ppnPassengerNode = _GetPassengerNode(ppl, ppPassenger);

   /* If passenger exists in list, the function executes */
	if(ppnPassengerNode != NULL){

      /* Checks if user attempted to change to the same seat that was already stored */
      if(ppnPassengerNode->iSeatNumber == iNewSeat){
         printf("%s already has that seat.\n", ppPassenger->pszName);
         ppnPassengerNode = NULL;
         return ERROR;
      }

      /* Stores the seat of the original node */
      iOriginalSeat = ppnPassengerNode->iSeatNumber;
      _DestroyPassengerNode(ppnPassengerNode);
      ppnPassengerNode = NULL;

      /* Removes the passenger */
		if((iStatus = _RemovePassenger(ppl, ppPassenger)) == ERROR){
         _DestroyPassengerNode(ppnPassengerNode);
         return iStatus;
      } 

      /* Adds the passenger again (with new seat number) */
	   if((iStatus = _AddPassengerNode(ppl, iNewSeat, ppPassenger)) == 1){

         /* If failed (seat was taken, seat was out of range) add passenger back to their original seat */
         _AddPassengerNode(ppl, iOriginalSeat, ppPassenger);
         return iStatus;
      } 

   /* If not, the function exits and prints and error. */
	} else {
		printf("Passenger doesn't exist.\n");	
      _DestroyPassengerNode(ppnPassengerNode);
      return ERROR;
	}

   /* Cleanup */
	ppnPassengerNode = NULL;
   return iStatus;
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
   
   };

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
      printf("Invalid flight id! Needs to be 4 digits/characters");
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

   return pfnCurrent;
}

/*
 * Destroys a FLIGHT_NODE * allocated by CreateFlight.
 * */
static int _DestroyFlight(FLIGHT_NODE *pfn){
   int iPListDestroyed;

   if(pfn == NULL){
      berror("Flight list is NULL. Cannot destroy unintialized list.");
      return -1; 
   }

   if((iPListDestroyed = _DestroyPassengerList(pfn->pfdData->pplPassengers)) == 1){
      berror("Could not destroy passenger list.");
      return -1;
   };

   free(pfn->pfdData->pszDestination);
   free(pfn->pfdData);
   free(pfn);

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
 * If any flights have that destination the number (n) is returned. Otherwise it returns -1.
 * */
int GetFlightNumberByDestination(FLIGHT_LIST *pfl, char szDestination[]){
   FLIGHT_NODE *pfnCurrent = NULL;
   int iPosition = 1; /* Starts at 1 */

   /* Checks if list if empty */
   if(pfl->pfnHead == NULL || pfl->pfnTail == NULL){
      berror("Can't search on an empty list.\n");
      return -1;
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
         pfnCurrent = NULL;
         return iPosition;
      }

      pfnCurrent = pfnCurrent->pfnNext;
      iPosition++;
   }

   /* Returns invalid value if nothing was found */
   return -1;
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
      berror("Position given is out of bounds.\n");
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
 * Inserts a new flight at the head of the list 
 * */
int AddFlight(FLIGHT_LIST *pfl, char szID[], int iDepartureTime, char szDestination[]){
   /* Declaring variables */
   int iStatusCode = ERROR;
   FLIGHT_NODE *pfNew = NULL;

   /* Check if flight exists on that ID */
   pfNew = _GetFlightByID(pfl, szID);
   if(pfNew != NULL){
      printf("Flight already exists!\n\n");
      pfNew = NULL;
      return 1;
   }

   /* Creates a new flight pointer */
   pfNew = _CreateFlight(szID, iDepartureTime, szDestination);

   /* If the pointer was created successfully, add it to the flight list */
   if (pfNew != NULL){
      /* If head is undefined (list is empty), set new node as head and tail */
      if(pfl->pfnHead == NULL){
         pfl->pfnHead = pfNew;
         pfl->pfnTail = pfNew;
         iStatusCode = OK;

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
      iStatusCode = OK;
   }

   /* Cleanup */
   pfNew = NULL;
   return iStatusCode;
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
static int _AddUniquePassenger(FLIGHT_LIST *pfl, char szName[], int iAge){
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
         return ERROR;
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
 * Adds a passenger to a flight (given its FlightId(sz)). 
 * Makes sure the passenger * is unique through the _GetUniquePassenger function.
 * */
int AddPassengerToFlight(FLIGHT_LIST *pfl, char szFlightID[], int iSeatNumber, char szName[], int iAge){
   /* Declaring pointers */
   FLIGHT_NODE *pfnFlight = NULL;
   PASSENGER *ppNewPassenger = NULL;
   int iAddedPassenger;

   /* TODO: use this somewhere?
   int _FlightIsEmpty(FLIGHT_LIST *pfl, char szID[]){
   */

   /* Retrieves a flight by its ID */
   bdebug("_GetFlightByID()\n");
   pfnFlight = _GetFlightByID(pfl, szFlightID);
   if(pfnFlight == NULL){
      printf("No flight exists on that ID.\n");
   }

   /* Checks seat is within bounds */
   if(iSeatNumber > MAX_SEATS || iSeatNumber < 0){
      printf("%d is not a valid seat number. Needs to be a number between 0 and %d\n", iSeatNumber, MAX_SEATS);
      return ERROR;
   }

   /* Attempts to add unique passenger to the list. Does not add if person already exists. */
   bdebug("_AddUniquePassenger()\n");
   if(_AddUniquePassenger(pfl, szName, iAge) == 0){
      bdebug("Added unique passenger.\n");
   };

   bdebug("_GetUniquePassenger()\n");
   /* Retrieves unique passenger of that name from the list. 
    NOTE: If two passengers have the same name, the last one added will be returned */
   ppNewPassenger = _GetUniquePassenger(pfl, szName);

   /* Attempts to add passenger to list */
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
 * Changes the seat of a passenger, Function is really similar to the previous one so 
 * I mainly comment on the differences.
 * */
int ChangePassengerSeat(FLIGHT_LIST *pfl, char szFlightID[], char szName[], int iNewSeat){
   PASSENGER *ppPassenger = NULL;
   FLIGHT_NODE *pfnFlight = NULL;
   int iSeatChanged;

   if(iNewSeat > MAX_SEATS || iNewSeat < 0){
      printf("%d is not a valid seat number. Needs to be a number between 0 and %d\n", iNewSeat, MAX_SEATS);
      return ERROR;
   }

   pfnFlight = _GetFlightByID(pfl, szFlightID);
   if(pfnFlight == NULL){
      printf("No flight exists on that ID.\n");
   }

   ppPassenger = _GetUniquePassenger(pfl, szName);
   /* NOTE: We did not add a new passenger like we did in the last function*/

   /* Attempt to change the seat of the passenger with the given name */
   iSeatChanged = _ChangeSeat(pfnFlight->pfdData->pplPassengers, ppPassenger, iNewSeat);
   

   ppPassenger = NULL;
   pfnFlight = NULL;

   return iSeatChanged;
}

/*
 * Removes and deletes a flight from the flight list
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
 * Prints a list of passengers for a flight by its Flight ID.
 * */
int PrintPassengers(FLIGHT_LIST *pfl, char szFlightID[]){
   if(pfl->iLength == 0){
      printf("-> no flights have been added to the list\n");
      return ERROR;
   }

   FLIGHT_NODE *pfnFlight = NULL;

   pfnFlight = _GetFlightByID(pfl, szFlightID);
   if(pfnFlight == NULL){
      return ERROR;
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
   printf("%d-> ", n);
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
void PrintFlightListSimple(FLIGHT_LIST *pfl){
   FLIGHT_NODE *pfnCurrent = NULL; 
   int n = 1;

   if(pfl->iLength == 0){
      printf("No flights have been added to the list\n");
      return;
   }

   /* Starts at HEAD, stores the flights to be printed */
   pfnCurrent = pfl->pfnHead;

   while(pfnCurrent != NULL){
      printf("%d->", n);   
      _PrintFlightSimple(pfnCurrent);

      /* Checks next flight */
      n++;
      pfnCurrent = pfnCurrent->pfnNext;
   }

   /* cleanup */
   pfnCurrent = NULL;

   return;
}

/*
 * Gets a list of every flight a passenger is booked to, returns the number of flights
 * */
int GetPassengersFlights(FLIGHT_LIST *pfl, char szName[]){
   /* Declare variables */
   FLIGHT_NODE *pfnCurrentFlight = NULL;
   PASSENGER_NODE *ppnPassenger = NULL;
   int iFlightsFound = 0;

   /* Checks if passenger exists */
   if(_PassengerExists(pfl, szName) != 0){
      /* Since the passenger has no flights, we return the initialized value (0) */
      return iFlightsFound;
   };
   
   /* Traverse the whole flight list, printing every flight that the passenger is a part of */
   pfnCurrentFlight = pfl->pfnHead;
   while(pfnCurrentFlight != NULL){

      /* Traverse the passenger list, printing when finding match to input name */
      ppnPassenger = pfnCurrentFlight->pfdData->pplPassengers->ppnHead;
      while(ppnPassenger != NULL){
         /* Since we verified the name with _PassengerExists we can just refer with the name here */ 
         if(strcmp(ppnPassenger->ppPassenger->pszName, szName) == 0){
            iFlightsFound++;
            if(iFlightsFound > 1){
               _PrintFlightSimple(pfnCurrentFlight);
            }
         }
         ppnPassenger = ppnPassenger->ppnNext;
      }
      pfnCurrentFlight = pfnCurrentFlight->pfnNext;
   }

   /* Cleanup */
   pfnCurrentFlight = NULL;
   ppnPassenger = NULL;

   return iFlightsFound;
}

/*
 * Checks every flight and prints every passenger that is booked to more than one flight.
 * Returns 1 if no valid passengers are found, or 0 if someone is found.
 * */
int PrintPassengersWithMultipleFlights(FLIGHT_LIST *pfl){
   /* Declaring variables and initializing pointers */
   int i, iNumberOfFlights, iPersonPrinted;
   FLIGHT_NODE *pfnCurrentFlight = NULL;
   PASSENGER_NODE *ppnCurrentPassenger = NULL;

   /* If these two trackers are low enough there is no point in executing */
   if(pfl->iUniquePassengers == 0){
      printf("No passengers added.");
      return 1;
   }

   if(pfl->iLength <= 1){
      printf("Not enough flights added.");
      return 1;
   }
   
   /* The return value. If no people are printed we return 0. */
   iPersonPrinted = 0;
   /* Checks for every unique passenger */
   for(i = 0; i < pfl->iUniquePassengers; i++){
      /* Initialize to 0, will person is on flight if match is found */
      iNumberOfFlights = 0;

      /* Starting at head, check every flight ... */
      pfnCurrentFlight = pfl->pfnHead;
      while(pfnCurrentFlight != NULL){
         /* then start at head of that passenger list, and check every passenger for a match */
         ppnCurrentPassenger = pfnCurrentFlight->pfdData->pplPassengers->ppnHead;
         while(ppnCurrentPassenger != NULL){
            /* If we find a match we increment the tracker */
            if(_ComparePassengers(pfl->arrppUniquePassengers[i], ppnCurrentPassenger->ppPassenger) == 0){
               iNumberOfFlights++;
            }
      
            ppnCurrentPassenger = ppnCurrentPassenger->ppnNext;
         }
         
         pfnCurrentFlight = pfnCurrentFlight->pfnNext;

         /* If a person was has more than 1 flight we print, and edit the return value to indicate this */
         if(iNumberOfFlights > 1){
            printf("-> %s is booked to %d flights!\n", ppnCurrentPassenger->ppPassenger->pszName, iNumberOfFlights);
            iPersonPrinted = 1;
         }
      }
   }
   return iPersonPrinted;
}
