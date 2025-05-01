#include "flight_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "passenger_list.h"

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

FLIGHT_DATA *GetFlightData(FLIGHT_LIST fl, int n){
   return _GetFlight(fl, n)->pfdData; 
}
