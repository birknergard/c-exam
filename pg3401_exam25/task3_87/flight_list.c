#include "flight_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
//#include "passenger_list.h"

/*
 * TODO:
 * */
static FLIGHT *_CreateFlight(char szID[], int iDepartureTime, char szDestination[]){
   FLIGHT *pfCreated = NULL;

   /* Verifying string length for destination */
   if(strlen(szDestination) >= MAX_DESTINATION){
      berror("Provided destination exceeds max character count for destinations (%d)", MAX_DESTINATION);
      return NULL;
   }

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
   memset(pfCreated->pfdData, 0, sizeof(FLIGHT_DATA));
   memset(pfCreated->pfdData->iSeats, 0, sizeof(FLIGHT_DATA));
   
   /* Set data struct members */
   pfCreated->pfdData->iDepartureTime = iDepartureTime;

   strncpy(pfCreated->pfdData->szID, szID, MAX_ID);
   pfCreated->pfdData->szID[MAX_ID] = '\0';

   /* Set data struct members */
   strncpy(pfCreated->pfdData->szDestination, szDestination, strlen(szDestination));
   pfCreated->pfdData->szID[MAX_ID] = '\0';

   /* Stores size of data */
   pfCreated->iSize = sizeof(FLIGHT_DATA);

   /* Creates the passenger list within the flight data */
   pfCreated->pfdData->pplPassengers = CreatePassengerList();
   if(pfCreated->pfdData->pplPassengers == NULL){
      berror("Failed to create passenger list.");
      free(pfCreated->pfdData);
      free(pfCreated);
      pfCreated = NULL;
   }

   /* If nothing went wrong, returns the new Flight List by address */
   return pfCreated;
}

/*
 * TODO:
 * */
static int DestroyFlight(FLIGHT *pf){
   int iPListDestroyed;

   if(pf == NULL){
      berror("Flight list is NULL. Cannot destroy unintialized list.");
      return ERROR; 
   }

   if((iPListDestroyed = DestroyPassengerList(pf->pfdData->pplPassengers)) == 1){
      berror("Could not destroy passenger list.");
      return ERROR;
   };

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
   memset(pflCreated, 0, sizeof(FLIGHT_LIST));
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
      if(pfCurrent->pfdData != NULL){
         DestroyPassengerList(pfCurrent->pfdData->pplPassengers);
         printf("Freeing node data\n");
         free(pfCurrent->pfdData);
         pfCurrent->pfdData = NULL;
      }
      
      pfTemp = pfCurrent;
      pfCurrent = pfCurrent->pfNext;
      printf("->Freeing node\n");
      free(pfTemp);
   }

   pfCurrent = NULL;
   pfTemp = NULL;

   ppfl->pfFirst = NULL; 
   ppfl->pfLast = NULL; 

   printf("Freeing list.\n");
   free(ppfl);

   return OK;
}

/*
 * TODO:: Update
 * */
int _GetFlightNumberByDestination(FLIGHT_LIST fl, char szDestination[]){
   FLIGHT *pfCurrent = NULL;
   int iPosition = 1; /* Starts at 1 */

   /* Checks if list if empty */
   if(fl.pfFirst == NULL || fl.pfLast == NULL){
      berror("Can't search on an empty list.\n");
      return -1;
   }

   /* Verifies that input is within MAX_DESTINATION buffer */
   if(strlen(szDestination) >= MAX_DESTINATION){
      berror("Provided destination exceeds character limit (%d).\n", MAX_DESTINATION);
      return -1;
   }

   pfCurrent = fl.pfFirst;

   /* Searches the list until it reaches a null pointer (end) */
   while(pfCurrent != NULL){
      /* Since we checked the character length of szDestination beforehand, strcmp is safe(ish) */
      if(strcmp(pfCurrent->pfdData->szDestination, szDestination) == 0){
         return iPosition;
      }

      pfCurrent = pfCurrent->pfNext;
      iPosition++;
   }

   /* Returns invalid value if nothing was found */
   return -1;
}

/*
 * TODO:: Update
 * */
static FLIGHT *_GetFlightByID(FLIGHT_LIST fl, char szID[]){
   FLIGHT *pfCurrent = NULL;

   if(fl.pfFirst == NULL || fl.pfLast == NULL){
      berror("Can't search on an empty list.\n");
      return NULL;
   }

   pfCurrent = fl.pfFirst;

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
 * TODO:: Update
 * */
FLIGHT_DATA *GetFlightDataByPosition(FLIGHT_LIST fl, int n){
   int i;
   FLIGHT *pfCurrent = NULL;

   if(n >= fl.iLength || n < 0){
      berror("Position given is out of bounds.\n");
      return NULL;
   }

   if(n == 0){
      if(fl.pfFirst == NULL){
         berror("Flight list HEAD is not defined.\n");
         return NULL;
      }
      return fl.pfFirst->pfdData;
   }

   if(n == fl.iLength){
      if(fl.pfLast == NULL){
         berror("Flight list TAIL is not defined.\n");
         return NULL;
      }
      return fl.pfLast->pfdData; 
   }

   /* If index is smaller or equal to middle, Iterate forward from head ... 
   NOTE: If number is odd the number is automatically rounded down to nearest whole */
   if(n <= fl.iLength / 2){
      pfCurrent = fl.pfFirst;
      for(i = 0; i < n; i++){
         pfCurrent = pfCurrent->pfNext;
      }

   /* ... Else go backwards from tail */
   } else {
      pfCurrent = fl.pfLast;
      for(i = 0; i < n; i++){
         pfCurrent = pfCurrent->pfPrev;
      }

   } 

   return pfCurrent->pfdData;
}

/*
 * TODO:: Update
 * */
int AddFlight(FLIGHT_LIST *pfl, char szID[], int iDepartureTime, char szDestination[]){
   int iStatusCode = ERROR;
   FLIGHT *pfNew = _CreateFlight(szID, iDepartureTime, szDestination);

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
         iStatusCode = OK;
      }
   }

   pfl->iLength++;
   return iStatusCode;
}

/*
 *
 * */
int RemoveFlight(FLIGHT_LIST *pfl, char szID[]){
   FLIGHT *pfTarget = NULL;
   FLIGHT *pfAdjacentLeft = NULL;
   FLIGHT *pfAdjacentRight = NULL;
	int iStatus = ERROR;

   /* Checks list length */
   if(pfl->iLength == 0 || pfl->pfFirst == NULL){
       printf("You can't remove from an empty list.\n");
       return ERROR;
   }

   if((pfTarget = _GetFlightByID(*pfl, szID)) == NULL){
      printf("Flight with ID -> %d does not exist.\n", szID); 
      return ERROR;
   }

   /* Finds the adjacent nodes */
   pfAdjacentLeft = pfTarget->pfNext;
   pfAdjacentRight = pfTarget->pfPrev;

   /* Points the adjacent nodes to each other, omitting the target */
   pfAdjacentRight->pfNext = pfAdjacentLeft;
   pfAdjacentLeft->pfPrev = pfAdjacentRight;

   /* Detach target */
   pfTarget->pfNext = NULL;
   pfTarget->pfPrev = NULL;

   /* Destroys the node (See function definition) */
   DestroyFlight(pfTarget);

   /* Decrements the list counter */
   pfl->iLength--;

   pfTarget = NULL;
   pfAdjacentLeft = NULL;
   pfAdjacentRight = NULL;

   return iStatus;
}


void PrintPassengers(FLIGHT_DATA *pfd){
   PrintPassengerList(pfd->pplPassengers);
}
/*
 * TODO:: Create
 * */
void PrintFlightList(FLIGHT_LIST *pfl) {
   int n;
   FLIGHT_DATA *pfCurrentFlight; 

   pfCurrentFlight = pfl->pfFirst->pfdData;

   printf("List of Flights, size %d", pfl->iLength);
   while(pfCurrentFlight != NULL){
      printf("%d-> ", n);
      printf("%s - Destination: %s - Departs at: %d", 
             pfCurrentFlight->szID,
             pfCurrentFlight->szDestination,
             pfCurrentFlight->iDepartureTime

      );

      /* Inserts a little pointer to the head and tail :) */
      if(pfCurrentFlight == pfl->pfFirst->pfdData || pfCurrentFlight == pfl->pfLast->pfdData) puts("  <--\n");
      else puts("\n");

      /* Prints number of available seats */
      printf("Available seats: %d", (64 - pfCurrentFlight->pplPassengers->iLength));
      PrintPassengers(pfCurrentFlight);
   }

   /* Cleanup */
   pfCurrentFlight = NULL;

   return;
}

void InternalFlightListTest(){
   return;
}
