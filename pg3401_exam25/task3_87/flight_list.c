#include "flight_list.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

/*
 *
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
   pfCreated->pfdData->pplPassengers = CreatePassengerList();
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

   if((iPListDestroyed = DestroyPassengerList(pf->pfdData->pplPassengers)) == 1){
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

   /* Initializing pointer */
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

   return AddPassenger(pf->pfdData->pplPassengers, iSeatNumber, szName, iAge);
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

   return ChangeSeat(pf->pfdData->pplPassengers, szName, iNewSeat);
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
   PrintPassengerList(pfFlight->pfdData->pplPassengers);
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
   PrintPassengerList(pfFlight->pfdData->pplPassengers); /* See "passenger_list.h" for definition */

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

void PrintFlightListSimple(FLIGHT_LIST *pfl){
   int n;
   FLIGHT *pfCurrent; 
   FLIGHT_DATA *pfdData;

   if(pfl->iLength == 0){
      printf("No flights have been added to the list\n");
      return;
   }

   pfCurrent = pfl->pfFirst;

   for(n = 1; n <= pfl->iLength; n++){
      if(pfCurrent == NULL){
         break;
      }

      pfdData = pfCurrent->pfdData;
      printf(
         "%d-> #%s,  %s,  departs at %d",
         n, pfdData->szID, pfdData->pszDestination, pfdData->iDepartureTime
      ); 
      printf(", %d passengers on flight\n", pfdData->pplPassengers->iLength);
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
   int n, iFlightPrinted = 0;

   if(pfl == NULL){
      return ERROR;
   }

   /* Trading innefficiency (GetFlightByPosition is O(n)) with code clarity and error handling by invoking the getter i times */
   for(n = 1; n <= pfl->iLength; n++){
      pfCurrentFlight = GetFlightByPosition(pfl, n);
      ppdPassenger = GetPassengerData(pfCurrentFlight->pfdData->pplPassengers, szPassengerName);
      if(ppdPassenger != NULL){
         iFlightPrinted += PrintFlight(pfl, n);
      }
   }

   if(iFlightPrinted == 0) return ERROR;

   return OK;
}
