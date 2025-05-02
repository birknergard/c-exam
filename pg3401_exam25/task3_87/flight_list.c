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
   int iDestinationSize = strlen(szDestination);
   int iIDSize = strlen(szID);

   /* Verifying string length for destination */
   if(iDestinationSize >= MAX_DESTINATION || iDestinationSize < 1){
      berror("Provided destination(%d) exceeds max character count for destinations (%d)", iDestinationSize, MAX_DESTINATION);
      return NULL;
   }

   if(iIDSize != 4){
      berror("Invalid ID. Needs to be %d characters long\n", MAX_ID - 1);
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
      printf("->Freeing node\n");
      DestroyFlight(pfTemp);
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

      /* Since we checked the character length of szDestination beforehand, strcmp is safe */
      if(strcmp(pfCurrent->pfdData->pszDestination, szDestination) == 0){
         return iPosition;
      }

      pfCurrent = pfCurrent->pfNext;
      iPosition++;
   }

   /* Returns invalid value if nothing was found */
   return -1;
}

/*
 *
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
//
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
int RemoveFlight(FLIGHT_LIST *pfl, char szID[]){
   FLIGHT *pfTarget = NULL;
   FLIGHT *pfAdjacentLeft = NULL;
   FLIGHT *pfAdjacentRight = NULL;

   /* Checks list length */
   if(pfl->iLength == 0 || pfl->pfFirst == NULL){
       printf("You can't remove from an empty list.\n");
       return ERROR;
   }

   if((pfTarget = _GetFlightByID(*pfl, szID)) == NULL){
      printf("Flight with ID -> %d does not exist.\n", szID); 
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
   DestroyFlight(pfTarget);

   /* Decrements the list counter */
   pfl->iLength--;

   pfTarget = NULL;
   pfAdjacentLeft = NULL;
   pfAdjacentRight = NULL;

   return OK;
}

/*
 * TODO:: Create
 * */
void PrintFlightList(FLIGHT_LIST *pfl) {
   int n = 0;
   FLIGHT *pfCurrentFlight = NULL; 

   if(pfl->iLength == 0){
      printf("No flights have been added to the list\n");
      return;
   }

   pfCurrentFlight = pfl->pfFirst;

   printf("List of Flights, size %d\n", pfl->iLength);
   while(pfCurrentFlight != NULL){
      n++;
      printf("%d-> ", n);
      printf("%s: TO:  %s, DEPARTS: %d", 
             pfCurrentFlight->pfdData->szID,
             pfCurrentFlight->pfdData->pszDestination,
             pfCurrentFlight->pfdData->iDepartureTime

      );

      /* Inserts a little pointer to the head and tail :) */
      if(pfCurrentFlight == pfl->pfFirst && pfCurrentFlight == pfl->pfLast) printf("  <--HEAD/TAIL\n");
      else if(pfCurrentFlight == pfl->pfFirst) printf("  <--HEAD\n");
      else if(pfCurrentFlight == pfl->pfLast) printf("  <--TAIL\n");
      else puts("");

      /* Prints number of available seats */
      printf("   Available seats: %d\n", (MAX_SEATS - pfCurrentFlight->pfdData->pplPassengers->iLength));

      /* Prints the list of passengers for the flight */
      PrintPassengerList(pfCurrentFlight->pfdData->pplPassengers); /* See "passenger_list.h" for definition */
      
      pfCurrentFlight = pfCurrentFlight->pfNext;
   }

   /* Cleanup */
   pfCurrentFlight = NULL;

   return;
}

int InternalFlightListTest() {
   int iFailed = 0;
   int pos;
   char longDestination[MAX_DESTINATION + 10];
   FLIGHT_LIST *pfl;
   FLIGHT_DATA *data;

   bdebug("Starting internal flight list tests...\n");

   /* Test: Creating flight list */
   bdebug("Test: Creating flight list...");
   pfl = CreateFlightList();
   if (pfl == NULL) {
       bdebug("FAILED: Could not create flight list.\n");
       return 1;
   }

   /* Test: Adding valid flights */
   bdebug("Test: Adding flight FL001...");
   //PrintFlightList(pfl);
   if (AddFlight(pfl, "FL01", 900, "New York") != OK) {
       bdebug("FAILED: Could not add FL001.\n");
       iFailed = 1;
   }

   bdebug("Test: Adding flight FL002...");
   //PrintFlightList(pfl);
   if (AddFlight(pfl, "FL02", 1300, "London") != OK) {
       bdebug("FAILED: Could not add FL002.\n");
       iFailed = 1;
   }

   bdebug("Test: Adding flight FL003...");
   //PrintFlightList(pfl);
   if (AddFlight(pfl, "FL03", 1700, "Tokyo") != OK) {
       bdebug("FAILED: Could not add FL003.\n");
       iFailed = 1;
   }

   /* Test: Adding flight with too-long destination */
   bdebug("Test: Attempting to add flight with destination exceeding MAX_DESTINATION...");
   //PrintFlightList(pfl);
   memset(longDestination, 'A', MAX_DESTINATION + 9);
   longDestination[MAX_DESTINATION + 9] = '\0';
   if (AddFlight(pfl, "FL04", 1800, longDestination) != ERROR) {
       bdebug("FAILED: Accepted a destination exceeding MAX_DESTINATION.\n");
       iFailed = 1;
   }

   /* Test: Get flight number by valid destination */
   //PrintFlightList(pfl);
   bdebug("Test: Retrieving flight position by destination 'London'...");
   pos = _GetFlightNumberByDestination(*pfl, "London");
   if (pos != 2) {
       bdebug("FAILED: Expected position 2 for 'London', got %d\n", pos);
       iFailed = 1;
   }

   /* Test: Get flight number by invalid destination */
   //PrintFlightList(pfl);
   bdebug("Test: Retrieving flight position by destination 'Mars' (non-existent)...");
   pos = _GetFlightNumberByDestination(*pfl, "Mars");
   if (pos != -1) {
       bdebug("FAILED: Expected -1 for non-existent destination, got %d\n", pos);
       iFailed = 1;
   }

   /* Test: Accessing flight data at out-of-bounds index */
   bdebug("Test: Accessing out-of-bounds flight list position...");
   //PrintFlightList(pfl);
   if (GetFlightDataByPosition(*pfl, pfl->iLength + 1) != NULL) {
       bdebug("FAILED: Accessed out-of-bounds position in flight list.\n");
       iFailed = 1;
   }

   /* Test: Getting flight data at position 0 */
   bdebug("Test: Getting flight data at position 0...");
   //PrintFlightList(pfl);
   data = GetFlightDataByPosition(*pfl, 0);
   if (data == NULL || strcmp(data->szID, "FL03") != 0) {
       bdebug("FAILED: Expected 'FL03' at position 0.\n");
       iFailed = 1;
   }

   /* Test: Attempting to remove non-existent flight */
   bdebug("Test: Attempting to remove non-existent flight 'NONEXISTENT'...");
   //PrintFlightList(pfl);
   if (RemoveFlight(pfl, "NONEXISTENT") != ERROR) {
       bdebug("FAILED: Removed a non-existent flight.\n");
       iFailed = 1;
   }

   /* Test: Removing valid flight */
   bdebug("Test: Removing flight 'FL02'...");
   //PrintFlightList(pfl);
   if (RemoveFlight(pfl, "FL02") != OK) {
       bdebug("FAILED: Could not remove existing flight 'FL02'.\n");
       iFailed = 1;
   }

   /* Confirm removal */
   bdebug("Test: Confirming 'FL02' was removed...");
   //PrintFlightList(pfl);
   pos = _GetFlightNumberByDestination(*pfl, "London");
   if (pos != -1) {
       bdebug("FAILED: Flight 'London' should have been removed.\n");
       iFailed = 1;
   }

   /* Test: Removing valid flight */
   bdebug("Test: Removing flight 'FL03'...");
   if (RemoveFlight(pfl, "FL03") != OK) {
       bdebug("FAILED: Could not remove existing flight 'FL02'.\n");
       iFailed = 1;
   }
   bdebug("Test: Confirming 'FL03' was removed...");
   PrintFlightList(pfl);
   pos = _GetFlightNumberByDestination(*pfl, "London");
   if (pos != -1) {
       bdebug("FAILED: Flight 'Tokyo' should have been removed.\n");
       iFailed = 1;
   }

   /* Test: Destroying flight list */
   bdebug("Test: Destroying flight list...");
   if (DestroyFlightList(pfl) != OK) {
       bdebug("FAILED: Could not destroy flight list.\n");
       iFailed = 1;
   }

   bdebug("Finished internal flight list tests. %s\n", iFailed ? "There were errors." : "All tests passed.");
   return iFailed;
}
