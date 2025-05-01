#include "flight_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

static FLIGHT *_CreateFlight(const FLIGHT_DATA fd){
   FLIGHT *pfCreated = NULL;

   pfCreated = (FLIGHT *) malloc(sizeof(FLIGHT));
   if (pfCreated == NULL) {
      berror("failed malloc in CreateFlight()");
      return NULL;
   }

   memset(pfCreated, 0, sizeof(FLIGHT));

   pfCreated->pfdData = (FLIGHT_DATA *) malloc(sizeof(FLIGHT_DATA));
   if(pfCreated->pfdData == NULL){
      berror("Failed malloc in CreateFlight() struct fdData\n");
      free(pfCreated);
      pfCreated = NULL;
      return NULL;
   }

   pfCreated->iSize = sizeof(fd);
   memcpy(pfCreated->pfdData, &fd, pfCreated->iSize);

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
   pflCreated->pfHead = NULL;
   pflCreated->pfTail = NULL;
   
   /* Debugging for checking initial list length
   bdebug("Created new list. Length %d", pList->iLength);
   */
   
   return pflCreated;
}

int DestroyFlightList(FLIGHT_LIST **ppfl){
   FLIGHT *pfCurrent = (*ppfl)->pfHead;   
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

   (*ppfl)->pTail = NULL; 
   (*ppfl)->pHead = NULL; 

   printf("Freeing list.\n");
   free(*ppfl);

   return OK;
}

int AddFlight(FLIGHT_LIST *pfl, FLIGHT_DATA fd){
   int iStatusCode = ERROR;
   FLIGHT *pfNew = CreateNode(fd);

   if (pfNew != NULL){

      /* If head is undefined (list is empty), set new node as head and tail */
      if(pfl->pfHead == NULL){
         pfl->pfHead = pThis;
         pfl->pfTail = pThis;

         iStatusCode = OK;

      } else {
         /*  Newnode next ptr to current head */
         pfl->pfNext = pfl->pfHead;
         /*  set current head prev ptr to new node */
         pfl->pfHead->pfPrev = pfNew;
         /* Define new list head as new node */ 
         pfl->pfHead = pfNew;
         iStatusCode = OK;
      }
   }

   pfl->iLength++;
   return iStatusCode;
}

/* TODO:*/
int RemoveFlight(FLIGHT_LIST *pfl, int n){
   int iStatusCode = ERROR;

   return iStatusCode;
}

FLIGHT *GetFlight(FLIGHT_LIST fl, int n){
   int i;
   FLIGHT *pfCurrent = NULL;

   if(n >= fl.iLength || n < 0){
      berror("Position given is out of bounds.\n");
      return NULL;
   }

   if(n == 0){
      if(fl.pfHead == NULL){
         berror("Flight list HEAD is not defined.\n");
         return NULL;
      }
      return fl.pfHead;
   }

   if(n == fl.iLength){
      if(fl.pfTail == NULL){
         berror("Flight list TAIL is not defined.\n");
         return NULL;
      }
      return fl.pfTail; 
   }

   /* If index is smaller or equal to middle, Iterate forward from head ... 
   NOTE: If number is odd the number is automatically rounded down to nearest whole */
   if(n <= fl.iLength / 2){
      pfCurrent = fl.pfHead;
      for(i = 0; i < n; i++){
         pfCurrent = pfCurrent->pfNext;
      }

   /* ... Else go backwards from tail */
   } else {
      pfCurrent = fl.pfTail;
      for(i = 0; i < n; i++){
         pfCurrent = pfCurrent->pfPrev;
      }

   } 

   return pfCurrent;
}

FLIGHT_DATA *GetFlightData(FLIGHT_LIST fl, int n){
   return Get(fl, n)->pfdData; 
}
