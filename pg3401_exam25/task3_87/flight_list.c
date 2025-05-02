#include "flight_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "passenger_list.h"

/*
 * TODO:
 * */
static FLIGHT *_CreateFlight(const FLIGHT_DATA fd){
   FLIGHT *pfCreated = NULL;

   pfCreated = (FLIGHT *) malloc(sizeof(FLIGHT));
   if(pfCreated == NULL){
      berror("Failed malloc in CreateFlight()");
      return NULL;
   }

   memset(pfCreated, 0, sizeof(FLIGHT));

   pfCreated->pfdData = (FLIGHT_DATA *) malloc(sizeof(FLIGHT_DATA));
   if(pfCreated->pfdData == NULL){
      berror("Failed malloc in CreateFlight() struct fdData\n");
      free(pfCreated);
      pfCreated = NULL;
      return pfCreated;
   }

   pfCreated->pfNext = NULL;
   pfCreated->pfPrev = NULL;
   pfCreated->iSize = sizeof(fd);
   memcpy(pfCreated->pfdData, &fd, pfCreated->iSize);
   if(pfCreated->pfdData == NULL){
      berror("Failed memcpy in CreateFlight().");
      free(pfCreated->pfdData);
      free(pfCreated);
      pfCreated = NULL;
   }

   return pfCreated;
}

/*
 * TODO:
 * */
static int DestroyFlight(FLIGHT *pf){

}

FLIGHT_LIST *CreateFlightList(){
   FLIGHT_LIST *pflCreated = NULL;

   pflCreated = (FLIGHT_LIST *) malloc(sizeof(FLIGHT_LIST));
   if(pflCreated == NULL){
      bdebug("Failed malloc in CreateFlightList()");
      return NULL;
   }
   memset(pflCreated, 0, sizeof(FLIGHT_LIST));
   pflCreated->pfFirst = NULL;
   pflCreated->pfLast = NULL;
   
   /* Debugging for checking initial list length
   bdebug("Created new list. Length %d", pList->iLength);
   */
   
   return pflCreated;
}

/*
 * TODO:
 * */
int DestroyFlightList(FLIGHT_LIST **ppfl){
   FLIGHT *pfCurrent = (*ppfl)->pfFirst;   
   FLIGHT *pfTemp = NULL;

   while(pfCurrent != NULL){
      if(pfCurrent->pfdData != NULL){
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

   (*ppfl)->pfFirst = NULL; 
   (*ppfl)->pfLast = NULL; 

   printf("Freeing list.\n");
   free(*ppfl);

   return OK;
}

/*
 * TODO:
 * */
int AddFlight(FLIGHT_LIST *pfl, FLIGHT_DATA fd){
   int iStatusCode = ERROR;
   FLIGHT *pfNew = _CreateFlight(fd);

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

/* TODO:*/
int RemoveFlight(FLIGHT_LIST *pfl, char szFlightId[]){
   int iStatusCode = ERROR;

   return iStatusCode;
}

/*
 * TODO:
 * */
static FLIGHT *_GetFlight(FLIGHT_LIST fl, int n){
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
      return fl.pfFirst;
   }

   if(n == fl.iLength){
      if(fl.pfLast == NULL){
         berror("Flight list TAIL is not defined.\n");
         return NULL;
      }
      return fl.pfLast; 
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

   return pfCurrent;
}

/*
 * TODO:
 * */
FLIGHT_DATA *GetFlightData(FLIGHT_LIST fl, int n){
   return _GetFlight(fl, n)->pfdData; 
}

/*
 * TODO:
 * */
void PrintFlightList(FLIGHT_LIST *pfl) {

}

void InternalFlightListTest(){

    // Step 1: Create list
    FLIGHT_LIST *pfl = CreateFlightList();
    if (!pfl) {
        printf("Failed to create flight list.\n");
        return;
    }
    printf("Flight list created.\n");

    // Step 2: Add some flights
    FLIGHT_DATA fd1 = { "AA1", 100, 900, CreatePassengerList() };
    FLIGHT_DATA fd2 = { "BB2", 150, 1100, CreatePassengerList() };
    FLIGHT_DATA fd3 = { "CC3", 200, 1300, CreatePassengerList() };

    AddFlight(pfl, fd1);
    AddFlight(pfl, fd2);
    AddFlight(pfl, fd3);
    printf("Added 3 flights.\n");

    // Step 3: Print and check
    PrintFlightList(pfl);

    if (pfl->iLength != 3) {
        printf("Error: Expected flight list length 3, got %d\n", pfl->iLength);
    }

    // Step 4: Get by index
    FLIGHT_DATA *retrieved = GetFlightData(*pfl, 1);
    if (retrieved != NULL) {
        printf("Retrieved flight at index 1: ID=%s\n", retrieved->szFlightID);
    } else {
        printf("Failed to retrieve flight at index 1.\n");
    }

    // Step 5: Validate passengers list was allocated
    if (retrieved->pplPassengers == NULL) {
        printf("Error: Passenger list not initialized for flight %s\n", retrieved->szFlightID);
    } else {
        printf("Passenger list exists for flight %s\n", retrieved->szFlightID);
    }

    // Step 6: Remove a flight
    printf("Removing flight BB2...\n");
    int removeResult = RemoveFlight(pfl, "BB2");
    if (removeResult == OK) {
        printf("Flight BB2 removed.\n");
    } else {
        printf("Failed to remove flight BB2.\n");
    }

    // Step 7: Print and check state
    PrintFlightList(pfl);
    if (pfl->iLength != 2) {
        printf("Error: Expected flight list length 2 after removal, got %d\n", pfl->iLength);
    }

    // Step 8: Try invalid removal
    printf("Trying to remove non-existent flight ZZ9...\n");
    if (RemoveFlight(pfl, "ZZ9") != OK) {
        printf("Correctly handled removal of non-existent flight.\n");
    }

    // Step 9: Destroy list
    printf("Destroying flight list...\n");
    if (DestroyFlightList(&pfl) == OK) {
        printf("Flight list destroyed successfully.\n");
    } else {
        printf("Error destroying flight list.\n");
    }
}
