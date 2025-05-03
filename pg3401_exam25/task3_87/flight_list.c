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
 * Returns 0 (OK) if equal, else 1 (ERROR)
 * */
static int _ComparePassengers(PASSENGER *pp1, PASSENGER *pp2){
   /* If the struct values are equal, return OK */
   if(strcmp(pp1->pszName, pp2->pszName) == 0
   && pp1->iAge == pp1->iAge) return OK;

   /* else return ERROR */
   return ERROR;
}


static PASSENGER *_CreatePassenger(char szName[], int iAge){
   PASSENGER *ppNew = NULL; 
   
	ppNew = (PASSENGER_DATA *) malloc(sizeof(PASSENGER_DATA)); 
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
   int iNameLength = strlen(szName); 

	/* Checks that chosen name isn't too long */
	if(iNameLength >= MAX_NAME){
		printf("Chosen name is too long.\n");
		return NULL;
	}

	/* Allocate new passenger pointer */
	ppnNew = (PASSENGER_NODE *) malloc(sizeof(PASSENGER_NODE));
	if(ppnNew == NULL){
		berror("Failed malloc in _CreatePassenger()\n");
		return NULL;
	}

	/* Initialize it */
	memset(ppnNew, 0, sizeof(PASSENGER_NODE));
	ppnNew->ppdData = NULL;

   /* Defaults seat number to invalid number */
   ppnNew->iSeatNumber = iSeatNumber;
	ppnNew->ppnNext = NULL;

	/* Makes data field point to given passenger address */
	ppnNew->ppPassenger = ppPassenger; 
	if(ppnNew->ppdData == NULL){
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
   if(pd != NULL){
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
	free(pp);

	return OK;
}

/*
 * Retrieves a passenger node by passenger pointer. Passenger pointer is assigned later in the source file.
 * */
static PASSENGER_NODE *_GetPassengerNode(PASSENGER_LIST *ppl, PASSENGER *ppPassenger){
	PASSENGER_NODE *ppnCurrent = NULL;
	int iNewNameLength, iCurrentNameLength;

	/* Verifies list is not empty */
	if(ppl->iLength == 0){
		puts("Passenger list is empty.");
		return NULL;
	}

   /* If length of input is too high, assume passenger does not exist */
	iNewNameLength = strlen(szName);
	if(iNewNameLength > MAX_NAME){
		puts("Given name exceeds max character limit on names. Therefore it does not exist in list.");
		return NULL;
	}

	/* Start by checking the first passenger in the list */
	ppnCurrent = ppl->ppHead;	

   /* Checks list until hitting NULL pointer */
	while(ppnCurrent != NULL){

		/* If the current passenger has the same name we return the passenger */ 
		if(_ComparePassengers(ppnCurrent->ppPassenger, ppPassenger) == OK){
			printf("Passenger found!\n");
			return ppnCurrent;
		};

		/* If not we check the next one */
		ppnCurrent = ppnCurrent->ppNext;
	}

   /* Cleanup */
	ppnCurrent = NULL;
	return NULL;
}

/*
 * This internal function retrieves a passengers struct containing name and age. 
 * This is usd in conjunction with memcmp to find passenger nodes.
 * */
static PASSENGER *_GetPassenger(PASSENGER_LIST *ppl, PASSENGER *pp){
	PASSENGER_NODE *ppnNode = NULL;
	PASSENGER *ppPassenger = NULL;

   /* Attempts to find passenger in list */
	ppnNode = _GetPassengerNode(ppl, szName);

   /* Checks if a passenger was found, if not returns NULL */
	if(ppnNode == NULL){
		return NULL;
	}

	ppPassenger = ppnNode->ppPassenger;
	if(ppdData == NULL){
		berror("Passenger data was not found.\n");
		return NULL;
	}

	ppnNode = NULL;
	return ppPassenger; 
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
	pplNew->ppHead = NULL;

	return pplNew;
}


/*
 * Destroys a previously created passenger list.
 * Passenger data is not destroyed here as it is handled in its 
 *  own data structure later (FLIGHT_LIST.pdUniquePassengers)
 * */
static int _DestroyPassengerList(PASSENGER_LIST *ppl){

	/* Declaring variables */
	PASSENGER *ppnCurrent = NULL, *ppNext = NULL;

	/* Errors if attempting to destroy empty list */
	if(ppl == NULL){
		berror("Attempted to destroy an empty list.\n");
		return ERROR;
	}

   /* If ppnCurrent ends up being NULL it passes the while check, freeing the node
      Otherwise it enters the while */
	ppnCurrent = ppl->ppHead;

   /* Destroys every node in list */
	while(ppnCurrent != NULL){
		ppNext = ppnCurrent->ppNext;
		_DestroyPassengerNode(ppnCurrent);
		ppnCurrent = ppNext;
	}

   /* Cleanup */
	ppnCurrent = NULL;
	ppNext = NULL;
	free(ppl);
	return OK;
}

/*
 * Adds a new passenger node in increasing order of seat number (1->MAX)
 * */
static int _AddPassengerNode(PASSENGER_LIST *ppl, int iSeatNumber, PASSENGER *pp){
	PASSENGER_NODE *ppnNewPassenger = NULL;		
	PASSENGER_NODE *ppnCurrent = NULL;		
	PASSENGER_NODE *ppnPrev = NULL;		
	int iCompareResult, iStatus;


	/* Checks if passenger with that name already exists in the list */
	if(_GetPassengerNode(ppl, pp) != NULL){
		printf("Person already exists in list.\n");	
		return ERROR;
	}

	/* Allocates new passenger */
	ppnNewPassenger = _CreatePassengerNode(iSeatNumber, pp); 
	if(ppnNewPassenger == NULL){
		berror("Could not create add new passenger due to allocation error.\n");
		return ERROR;
	}

	/* If list is empty, inserts passenger in first position */
	bdebug("Adding passenger to list.\n");
	if(ppl->iLength == 0 || ppl->ppHead == NULL){
		ppl->ppHead = ppnNewPassenger;	
		ppl->iLength++;
		ppnNewPassenger = NULL;
		return OK;
	}

	/* Set current to first node */
	ppnCurrent = ppnl->ppHead;
	ppnPrev = ppnCurrent;

   /* Checks if node can be inserted at head */
	if (ppl->ppHead->iSeatNumber > iSeatNumber) {
		ppnNewPassenger->ppnNext = ppl->ppHead;
		ppl->ppHead = ppnNewPassenger;
		ppl->iLength++;

		ppnNewPassenger = NULL;		
		ppnCurrent = NULL;		
		ppnPrev = NULL;		
		return OK;
	}

	/* If not it continues until current reaches null pointer or finds correct adjacent node */
	while(ppnCurrent != NULL){
		iCompareResult = ppnCurrent->iSeatNumber < iSeatNumber;

		/* This should be prevented before this function is invoked. This is here as a precaution. */
		if(ppnCurrent->iSeatNumber == iSeatNumber){
			printf("Seat number %d is taken.\n", iSeatNumber);
			_DestroyPassenger(ppnNewPassenger);
			iStatus = ERROR;
			break;
		}

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
			ppnCurrent = ppnCurrent->ppNext;       

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
static int _RemovePassenger(PASSENGER_LIST *ppl, PASSENGER *ppData){
	PASSENGER *ppnCurrent = NULL;
	PASSENGER *ppnTarget = NULL;
	PASSENGER *ppnPrev = NULL;
	int iStatus = ERROR;

	if(ppl->iLength == 0){
		printf("You can't remove from an empty list.\n");
		return ERROR;
	}

	ppnCurrent = ppl->ppnHead;

	/* Checks head node first */
	if(_ComparePassengers(ppnCurrent->ppPassenger, ppData) == OK){
		bdebug("Deleting %s\n", szName);
		ppnTarget = ppnCurrent;
		ppl->ppnHead = ppnCurrent->ppnNext;  // Move head to next node

		/* Delete passenger */
		_DestroyPassenger(ppnTarget);
		ppl->iLength--;
		iStatus = OK;

	} else {
		/* Else traverse until the last */
		ppnPrev = ppnCurrent;
		ppnCurrent = ppnCurrent->ppNext;

		while(ppnCurrent != NULL){
			if(strncmp(szName, ppnCurrent->ppdData->pszName, MAX_NAME) == 0){
				bdebug("Deleting %s\n", szName);
				ppnTarget = ppnCurrent;
				ppnPrev->ppNext = ppnCurrent->ppNext;

				/* Deletes passenger */
				_DestroyPassenger(ppnTarget);
				ppl->iLength--;
				iStatus = OK;
				break;
			}

			ppnPrev = ppnCurrent; 
			ppnCurrent = ppnCurrent->ppNext;
		}
	}

   /* If target reaches null that means the node was not found */
	if(ppnTarget == NULL){
		printf("Could not find passenger named %s in list.\n", szName);
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
static int ChangeSeat(PASSENGER_LIST *ppl, PASSENGER *ppPassenger){
   /* Declares/initializes variables */
	PASSENGER_DATA *ppnPassengerNode = NULL;
	int iOriginalSeat, iStatus = OK;

   /* Retrieves a pointer to the passengers node */
	ppnPassengerNode = _GetPassengerNode(ppl, ppPassenger);

   /* If passenger exists in list, the function executes */
	if(ppnPassengerNode != NULL){

      /* Checks if user attempted to change to the same seat that was already stored */
      if(ppnPassengerNode->iSeatNumber == iNewSeat){
         printf("%s already has that seat.\n", ppPassenger->szName);

         ppnPassengerNode = NULL;
         return ERROR;
      }

      /* Stores the seat of the original node */
      iOriginalSeat = ppnPassengerNode->iSeatNumber;
      ppnPassengerNode = NULL;

      /* Removes the passenger */
		if((iStatus = _RemovePassenger(ppl, ppPassenger)) == ERROR){
         return iStatus;
      } 

      /* Adds the passenger again (with new seat number) */
	   if((iStatus = _AddPassenger(ppl, iNewSeat, ppPassenger)) == ERROR){
         /* Puts the passenger back to their original spot */
         _AddPassenger(ppl, iOriginalSeat, szName, iAge);
         return iStatus;
      } 

   /* If not, the function exits and prints and error. */
	} else {
		printf("Passenger doesn't exist.\n");	
      return ERROR;
	}

   /* Cleanup */
	ppnPassengerNode = NULL;
   return iStatus;
}

static void _PrintPassengerList(PASSENGER_LIST *ppl){
	PASSENGER_NODE *ppnCurrent = NULL;
	int n = 0;

	printf("   > PASSENGERS, %d people <\n", ppl->iLength);
	ppnCurrent = ppl->ppHead;

	while(ppnCurrent != NULL){
		n++;
		printf("   %d: %s, %d - SEAT:%d\n",
	 n,
	 ppnCurrent->ppdData->pszName,
	 ppnCurrent->ppdData->iAge
	 ppnCurrent->iSeatNumber,
	 );	
		ppnCurrent = ppnCurrent->ppNext;
	}
	puts("");

	ppnCurrent = NULL;
}

/*
 * FLIGHT LIST IMPLEMENTATION STARTS HERE
 * */
static FLIGHT *_CreateFlight(char szID[], int iDepartureTime, char szDestination[]){
   FLIGHT *pfCreated = NULL;
   int iDestinationSize = strlen(szDestination);
   int iIDSize = strlen(szID);

   /* Verifying string length for destination */
   if(iDestinationSize >= MAX_DESTINATION || iDestinationSize < 1){
      berror("Provided destination(%d) exceeds max character count for destinations (%d)", iDestinationSize, MAX_DESTINATION);
      return NULL;
   }

   if(iIDSize != 4){
      berror("Invalid ID (%d characters). Needs to be %d characters long\n", iIDSize,  MAX_ID - 1);
      return NULL;
   
   };

   /* Allocate new flight */
   pfCreated = (FLIGHT *) malloc(sizeof(FLIGHT));
   if(pfCreated == NULL){
      berror("Failed malloc in CreateFlight()");
      return NULL;
   }

   /* Initialize pointers to null */
   pfCreated->pfNext = NULL;
   pfCreated->pfPrev = NULL;

   /* Allocate data pointer */
   pfCreated->pfdData = (FLIGHT_DATA *) malloc(sizeof(FLIGHT_DATA));
   if(pfCreated->pfdData == NULL){
      berror("Failed malloc in CreateFlight() struct fdData\n");
      free(pfCreated);
      pfCreated = NULL;
      return pfCreated;
   }
   /* Initializing destination pointer to null*/
   pfCreated->pfdData->pszDestination = NULL;

   /* Initializing ID array*/
   memset(pfCreated->pfdData->szID, 0, MAX_ID);
   pfCreated->pfdData->pszDestination = (char *) malloc(iDestinationSize + 1);
   if(pfCreated->pfdData->pszDestination == NULL){
      berror("Failed malloc for creating szDestination.\n");
      free(pfCreated->pfdData);
      free(pfCreated);
      return NULL;
   }

   /* Set data struct members */
   pfCreated->pfdData->iDepartureTime = iDepartureTime;

   strncpy(pfCreated->pfdData->szID, szID, MAX_ID);
   pfCreated->pfdData->szID[MAX_ID - 1] = '\0';

   /* Set data struct members */
   strncpy(pfCreated->pfdData->pszDestination, szDestination, iDestinationSize);
   pfCreated->pfdData->pszDestination[iDestinationSize] = '\0';

   /* Stores size of data */
   pfCreated->iSize = sizeof(pfCreated->pfdData);

   /* Creates the passenger list within the flight data */
   pfCreated->pfdData->pplPassengers = _CreatePassengerList();
   if(pfCreated->pfdData->pplPassengers == NULL){
      berror("Failed to create passenger list.");
      free(pfCreated->pfdData->pszDestination);
      free(pfCreated->pfdData);
      free(pfCreated);
      pfCreated = NULL;
   }

   /* If nothing went wrong, returns the new Flight List by address */
   return pfCreated;
}

/*
 *
 * */
static FLIGHT *_GetFlightByID(FLIGHT_LIST *pfl, char szID[]){
   FLIGHT *pfCurrent = NULL;

   if(pfl->pfFirst == NULL || pfl->pfLast == NULL){
      berror("Can't search on an empty list.\n");
      return NULL;
   }

   if(strlen(szID) != 4){
      printf("Invalid flight id! Needs to be 4 digits/characters");
      return NULL;
   }

   pfCurrent = pfl->pfFirst;

   /* Check the rest */
   while(pfCurrent != NULL){
      if(strncmp(pfCurrent->pfdData->szID, szID, MAX_ID) == 0){
         return pfCurrent;
      }

      pfCurrent = pfCurrent->pfNext;
   }

   return pfCurrent;
}

/*
 *
 * */
static int _DestroyFlight(FLIGHT *pf){
   int iPListDestroyed;

   if(pf == NULL){
      berror("Flight list is NULL. Cannot destroy unintialized list.");
      return ERROR; 
   }

   if((iPListDestroyed = _DestroyPassengerList(pf->pfdData->pplPassengers)) == 1){
      berror("Could not destroy passenger list.");
      return ERROR;
   };

   free(pf->pfdData->pszDestination);
   free(pf->pfdData);
   free(pf);

   return OK;
}


/*
 *
 * */
FLIGHT_LIST *CreateFlightList(){
   FLIGHT_LIST *pflCreated = NULL;

   /* Allocating for flight list. */
   pflCreated = (FLIGHT_LIST *) malloc(sizeof(FLIGHT_LIST));
   if(pflCreated == NULL){
      bdebug("Failed malloc in CreateFlightList()");
      return NULL;
   }

   /* Allocates PASSENGER ** for holding passenger addresses */
   pflCreated->ppUniquePassengers = (PASSENGER **) malloc(sizeof(PASSENGER *));
   if(pflCreated->ppUniquePassengers == NULL){
      berror("Failed to allocate unique passenger pointer.");
      free(pflCreated);
      pflCreated = NULL;
      return NULL;
   }
   

   /* Initializing pointers */
   pflCreated->iLength = 0;
   pflCreated->pfFirst = NULL;
   pflCreated->pfLast = NULL;
   
   return pflCreated;
}

/*
 *
 * */
int DestroyFlightList(FLIGHT_LIST *ppfl){
   FLIGHT *pfCurrent = ppfl->pfFirst;   
   FLIGHT *pfTemp = NULL;

   /* Goes through the list, freeing each node and its corresponding data */
   while(pfCurrent != NULL){
      pfTemp = pfCurrent;
      pfCurrent = pfCurrent->pfNext;
      _DestroyFlight(pfTemp);
   }

   pfCurrent = NULL;
   pfTemp = NULL;

   ppfl->pfFirst = NULL; 
   ppfl->pfLast = NULL; 

   free(ppfl);

   return OK;
}

/*
 *
 * */
int FlightListIsEmpty(FLIGHT_LIST *pfl){
   if(pfl != NULL){
      return pfl->iLength == 0;
   } else {
      return -1; 
   }
}

/*
 *
 * */
int FlightIsEmpty(FLIGHT_LIST *pfl, char szID[]){
   FLIGHT *pfFlight = NULL;

   if(FlightListIsEmpty(pfl) != 0){
      pfFlight = _GetFlightByID(pfl, szID);   
      if(pfFlight == NULL){
         berror("flight does not exist.");
         return -1;
      }
      if(pfFlight->pfdData->pplPassengers != NULL){
         return pfFlight->pfdData->pplPassengers->iLength == 0;
      }
   } 

   return -1; 
}

/*
 *
 * */
int GetFlightNumberByDestination(FLIGHT_LIST *pfl, char szDestination[]){
   FLIGHT *pfCurrent = NULL;
   int iPosition = 1; /* Starts at 1 */

   /* Checks if list if empty */
   if(pfl->pfFirst == NULL || pfl->pfLast == NULL){
      berror("Can't search on an empty list.\n");
      return -1;
   }

   /* Verifies that input is within MAX_DESTINATION buffer */
   if(strlen(szDestination) >= MAX_DESTINATION){
      berror("Provided destination exceeds character limit (%d).\n", MAX_DESTINATION);
      return -1;
   }

   pfCurrent = pfl->pfFirst;

   /* Searches the list until it reaches a null pointer (end) */
   while(pfCurrent != NULL){

      /* Since we checked the character length of szDestination beforehand, strcmp is safe */
      if(strcmp(pfCurrent->pfdData->pszDestination, szDestination) == 0){
         pfCurrent = NULL;
         return iPosition;
      }

      pfCurrent = pfCurrent->pfNext;
      iPosition++;
   }

   /* Returns invalid value if nothing was found */
   return -1;
}


/*
 * Gets FLIGHT by position in list. Starts at 1 (not zero-indexed) 
 * */
FLIGHT *GetFlightByPosition(FLIGHT_LIST *pfl, int n){
   int i;
   FLIGHT *pfCurrent = NULL;

   if(n > pfl->iLength || n < 0){
      berror("Position given is out of bounds.\n");
      return NULL;
   }

   if(n == 1){
      if(pfl->pfFirst == NULL){
         berror("Flight list HEAD is not defined.\n");
         return NULL;
      }
      return pfl->pfFirst;
   }


   if(n == pfl->iLength){
      if(pfl->pfLast == NULL){
         berror("Flight list TAIL is not defined.\n");
         return NULL;
      }
      return pfl->pfLast; 
   }

   /* If index is smaller or equal to middle, Iterate forward from head ... 
   NOTE: If number is odd the number is automatically rounded down to nearest whole */
   if(n <= pfl->iLength / 2){
      pfCurrent = pfl->pfFirst;
      for(i = 0; i < n; i++){
         pfCurrent = pfCurrent->pfNext;
      }

   /* ... Else go backwards from tail */
   } else {
      pfCurrent = pfl->pfLast;
      for(i = 0; i < n; i++){
         pfCurrent = pfCurrent->pfPrev;
      }
   } 

   return pfCurrent;
}

/*
 * Inserts a new flight at the head of the list 
 * */
int AddFlight(FLIGHT_LIST *pfl, char szID[], int iDepartureTime, char szDestination[]){
   int iStatusCode = ERROR;
   FLIGHT *pfNew = NULL;

   pfNew = _CreateFlight(szID, iDepartureTime, szDestination);
   if (pfNew != NULL){
      /* If head is undefined (list is empty), set new node as head and tail */
      if(pfl->pfFirst == NULL){
         pfl->pfFirst = pfNew;
         pfl->pfLast = pfNew;
         iStatusCode = OK;

      } else {
         /*  Newnode next ptr to current head */
         pfNew->pfNext = pfl->pfFirst;

         /*  set current head prev ptr to new node */
         pfl->pfFirst->pfPrev = pfNew;

         /* Define new list head as new node */ 
         pfl->pfFirst = pfNew;

      }
      pfl->iLength++;
      iStatusCode = OK;
   }

   pfNew = NULL;
   return iStatusCode;
}

/*
 *
 * */
int AddPassengerToFlight(FLIGHT_LIST *pfl, char szFlightID[], int iSeatNumber, char szName[], int iAge){
   FLIGHT *pf = NULL;

   pf = _GetFlightByID(pfl, szFlightID);
   if(pf == NULL){
      printf("No flight exists on that ID.\n");
   }

   if(iSeatNumber > MAX_SEATS || iSeatNumber < 0){
      printf("%d is not a valid seat number. Needs to be a number between 0 and %d\n", iSeatNumber, MAX_SEATS);
      return ERROR;
   }

   return _AddPassenger(pf->pfdData->pplPassengers, iSeatNumber, szName, iAge);
}

/*
 *
 * */
int ChangePassengerSeat(FLIGHT_LIST *pfl, char szFlightID[], char szName[], int iNewSeat){
   FLIGHT *pf = NULL;

   if(iNewSeat > MAX_SEATS || iNewSeat < 0){
      printf("%d is not a valid seat number. Needs to be a number between 0 and %d\n", iNewSeat, MAX_SEATS);
      return ERROR;
   }

   pf = _GetFlightByID(pfl, szFlightID);
   if(pf == NULL){
      printf("No flight exists on that ID.\n");
   }

   return _ChangeSeat(pf->pfdData->pplPassengers, szName, iNewSeat);
}

/*
 *
 * */
int RemoveFlight(FLIGHT_LIST *pfl, char szID[]){
   FLIGHT *pfTarget = NULL;
   FLIGHT *pfAdjacentLeft = NULL;
   FLIGHT *pfAdjacentRight = NULL;

   /* Checks list length */
   if(pfl->iLength == 0 || pfl->pfFirst == NULL){
       printf("You can't remove from an empty list.\n");
       return ERROR;
   }

   if((pfTarget = _GetFlightByID(pfl, szID)) == NULL){
      printf("Flight with ID -> %s does not exist.\n", szID); 
      return ERROR;
   }

   /* Should be null if its the only item in the list, else should be another node */
   if(pfTarget == pfl->pfFirst){
      pfl->pfFirst = pfl->pfFirst->pfNext;   
   }

   if(pfTarget == pfl->pfLast){
      pfl->pfLast = pfl->pfLast->pfPrev;   
   }

   /* Finds the adjacent nodes (even if they are null) */
   pfAdjacentLeft = pfTarget->pfNext;
   pfAdjacentRight = pfTarget->pfPrev;

   /* Points the adjacent nodes to each other, omitting the target */
   if (pfAdjacentRight != NULL)
      pfAdjacentRight->pfNext = pfAdjacentLeft;
   if (pfAdjacentLeft != NULL)
      pfAdjacentLeft->pfPrev = pfAdjacentRight;

   /* Detach target */
   pfTarget->pfNext = NULL;
   pfTarget->pfPrev = NULL;

   /* Destroys the node (See function definition) */
   _DestroyFlight(pfTarget);

   /* Decrements the list counter */
   pfl->iLength--;

   pfTarget = NULL;
   pfAdjacentLeft = NULL;
   pfAdjacentRight = NULL;

   return OK;
}

int PrintPassengers(FLIGHT_LIST *pfl, char szFlightID[]){
   if(pfl->iLength == 0){
      printf("-> no flights have been added to the list\n");
      return ERROR;
   }

   FLIGHT *pfFlight = NULL;

   pfFlight = _GetFlightByID(pfl, szFlightID);
   if(pfFlight == NULL){
      return ERROR;
   } 

   /* Defined in passenger_list.h */
   _PrintPassengerList(pfFlight->pfdData->pplPassengers);
   return OK;
}

/*
 * Prints a single flight by its position (N)
 * */
int PrintFlight(FLIGHT_LIST *pfl, int n){
   if(pfl->iLength == 0){
      printf("-> no flights have been added to the list\n");
      return ERROR;
   }

   FLIGHT *pfFlight = NULL;

   pfFlight = GetFlightByPosition(pfl, n);
   if(pfFlight == NULL){
      return ERROR;
   }
   printf("%d-> ", n);
   printf("%s: TO:  %s, DEPARTS: %d", 
          pfFlight->pfdData->szID,
          pfFlight->pfdData->pszDestination,
          pfFlight->pfdData->iDepartureTime
   );

   /* Inserts a little pointer to the head and tail :) */
   if(pfFlight == pfl->pfFirst && pfFlight == pfl->pfLast) printf("  <--HEAD/TAIL\n");
   else if(pfFlight == pfl->pfFirst) printf("  <--HEAD\n");
   else if(pfFlight == pfl->pfLast) printf("  <--TAIL\n");
   else puts("");

   /* Prints number of available seats */
   printf("   Available seats: %d\n", (MAX_SEATS - pfFlight->pfdData->pplPassengers->iLength));

   /* Prints the list of passengers for the flight */
   _PrintPassengerList(pfFlight->pfdData->pplPassengers); /* See "passenger_list.h" for definition */

   return OK;
}

/*
 * 
 * */
void PrintFlightList(FLIGHT_LIST *pfl) {
   int n;

   if(pfl->iLength == 0){
      printf("-> no flights have been added to the list\n");
      return;
   }

   for(n = 1; n <= pfl->iLength; n++){
      if(PrintFlight(pfl, n) == ERROR){
         break;
      }
   }
   return;
}

/*
 *
 * */
static int _PrintFlightSimple(FLIGHT *pf){
   if(pf == NULL){
      return ERROR;
   }

   FLIGHT_DATA *pfdData = pf->pfdData;
   printf(
      "#%s,  %s,  departs at %d",
      pfdData->szID, pfdData->pszDestination, pfdData->iDepartureTime
   ); 
   printf(", %d passengers on flight\n", pfdData->pplPassengers->iLength);

   return OK;
}

/*
 *
 * */
void PrintFlightListSimple(FLIGHT_LIST *pfl){
   int n;
   FLIGHT *pfCurrent; 

   if(pfl->iLength == 0){
      printf("No flights have been added to the list\n");
      return;
   }

   pfCurrent = pfl->pfFirst;

   for(n = 1; n <= pfl->iLength; n++){
      if(pfCurrent == NULL){
         break;
      }

      _PrintFlightSimple(pfCurrent);

      pfCurrent = pfCurrent->pfNext;
   }


   pfCurrent = NULL;
   return;
}
/*
 * For use in option 7
 * */
int GetPassengersFlights(FLIGHT_LIST *pfl, char szPassengerName[]){
   FLIGHT *pfCurrentFlight = NULL;
   PASSENGER_DATA *ppdPassenger = NULL;
   int n, iFlightPrinted = ERROR;

   if(pfl == NULL){
      return ERROR;
   }

   /* Trading innefficiency (GetFlightByPosition is O(n)) with code clarity and error handling by invoking the getter i times */
   for(n = 1; n <= pfl->iLength; n++){
      pfCurrentFlight = GetFlightByPosition(pfl, n);
      ppdPassenger = _GetPassengerData(pfCurrentFlight->pfdData->pplPassengers, szPassengerName);
      if(ppdPassenger != NULL){
         iFlightPrinted = _PrintFlightSimple(pfCurrentFlight);
      }
   }

   return iFlightPrinted;
}

/*
int PrintPassengersWithMultipleFlights(FLIGHT_LIST *pfl){
    Find every unique passenger (names + age), and store their names in a list 
   PASSENGER_LIST *ppl = NULL;
   PASSENGER *ppPassenger = NULL;
   FLIGHT *pfCurrent = NULL;
   char *pszPassengerNames[MAX_NAME] = NULL;
   int i, j, iNamesAdded;

   / Set max unique passengers to 200 /
   pszPassengerNames = (char **) malloc(MAX_NAME * 200);
   if(pszPassengerNames == NULL){
      berror("Malloc failed.");
      return -1;
   } 

   iNamesAdded = 0;
   pfCurrent = pfl->pfFirst;
   for(i = 0; i < pfl->iLength; i++){
      ppl = pfCurrent->pfdData->pplPassengerList;
      ppnCurrent = ppl->ppHead;
      while(ppnCurrent != NULL){
         ppnCurrent = ppnCurrent->ppNext;;

         if(iNamesAdded == 0){
            strncpy(pszPassengerNames[i], ppnCurrent->ppdData->pszName);
         }
      }
      pfCurrent = pfCurrent->pfNext;
   }

   / Find how many flights each passenger is booked to (hint, GetPassengerFlights)/
   / If its more than 1, print the unique name /


}
*/
