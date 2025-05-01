#include "ll_double.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

static NODE *CreateNode(const void *pvData){
   NODE *pNode;

   pNode = (NODE *) malloc(sizeof(NODE));
   if (pNode == NULL) {
      berror("failed malloc in CreateNode()");
      return NULL;
   }

   memset (pNode, 0, sizeof(NODE));

   pNode->pvData = (void *) malloc(sizeof(void *));
   if(pNode->pvData == NULL){
      berror("Failed malloc in CreateNode() struct pvData\n");
      free(pNode);
      pNode = NULL;
      return NULL;
   }

   pNode->iSize = sizeof(*pvData);
   memcpy(pNode->pvData, pvData, sizeof(void *));

   return pNode;
}

LIST *CreateList(){
   LIST *pList;
   pList = NULL;

   pList = (LIST *) malloc(sizeof(LIST));
   if(pList == NULL){
      bdebug("Error with malloc in CreateList().");
      return NULL;
   }

   memset(pList, 0, sizeof(LIST));
   
   /* Debugging for checking initial list length
   bdebug("Created new list. Length %d", pList->iLength);
   */
   
   return pList;
}

int DestroyList(LIST **ppList){
   NODE *pCurrent = (*ppList)->pHead;   
   NODE *pTemp = NULL;

   while(pCurrent != NULL){
      if(pCurrent->pvData != NULL){
         printf("Freeing node data\n");
         free(pCurrent->pvData);
         pCurrent->pvData = NULL;
      }
      
      pTemp = pCurrent;
      pCurrent = pCurrent->pNext;
      printf("->Freeing node\n");
      free(pTemp);
   }

   pCurrent = NULL;
   pTemp = NULL;

   (*ppList)->pTail = NULL; 
   (*ppList)->pHead = NULL; 

   printf("Freeing list.\n");
   free(*ppList);

   return OK;
}

int Add(LIST *pList, void *pvData){
   int iStatusCode = ERROR;
   NODE *pThis = CreateNode(pvData);

   if (pThis != NULL){

      /* If head is undefined (list is empty), set new node as head and tail */
      if(pList->pHead == NULL){
         pList->pHead = pThis;
         pList->pTail = pThis;

         iStatusCode = OK;

      } else {
         /*  Newnode next ptr to current head */
         pThis->pNext = pList->pHead;
         /*  set current head prev ptr to new node */
         pList->pHead->pPrev = pThis;
         /* Define new list head as new node */ 
         pList->pHead = pThis;
         iStatusCode = OK;
      }
   }

   pList->iLength++;
   return iStatusCode;
}

int Append(LIST *pList, void *pvData){
   int iStatusCode = ERROR;
   NODE *pThis = CreateNode(pvData);
   if (pThis != NULL) {
      if (pList->pHead == NULL) {
         /* For nodeless list(?) */ 
         pList->pHead = pThis;
      }
      /* Set tail node next ptr to new node */
      pList->pTail->pNext = pThis;

      /* Set new node as tail */
      pList->pTail = pThis;

      iStatusCode = OK;
   }
   pList->iLength++;

   return iStatusCode;
}

int Remove(LIST *pList, int i){
   int iStatusCode = ERROR;

   return iStatusCode;
}

NODE *Get(LIST pList, int i){
   int j;
   NODE *pCurrent = NULL;

   if(i >= pList.iLength || i < 0){
      berror("Index out of bounds.\n");
      return NULL;
   }

   if(i == 0){
      if(pList.pHead == NULL){
         berror("List HEAD is not defined.\n");
         return NULL;
      }
      return pList.pHead;
   }

   if(i == pList.iLength - 1){
      if(pList.pTail == NULL){
         berror("List TAIL is not defined.\n");
         return NULL;
      }
      return pList.pTail; 
   }

   /* If index is smaller or equal to middle, Iterate forward from head ... 
   NOTE: If number is odd the number is automatically rounded down to nearest whole */
   if(i <= pList.iLength / 2){
      pCurrent = pList.pHead;
      for(j = 0; j < i; j++){
         pCurrent = pCurrent->pNext;
      }

   /* ... Else go backwards from tail */
   } else {
      pCurrent = pList.pTail;
      for(j = 0; j < i; j++){
         pCurrent = pCurrent->pPrev;
      }

   } 

   return pCurrent;
}

void *GetData(LIST pList, int i){
   return Get(pList, i)->pvData; 
}
