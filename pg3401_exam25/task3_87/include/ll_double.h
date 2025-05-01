/*--------------------------------------------------
* AUTHOR: BKN
* DESCRIPTION: Doubly linked list implementation
*              which holds anonymous datatype in
*              form of a void *. Hence it is made
*              to be used as a baseline for further
*              abstractions.
--------------------------------------------------*/
#ifndef ___LL_DOUBLE_H___ 
#define ___LL_DOUBLE_H___  

/* Node struct */
typedef struct _NODE {
   struct _NODE *pNext;
   struct _NODE *pPrev;
   int iSize;
   void *pvData;
} NODE;

/* Doubly linked list struct */
typedef struct _LL_DOUBLE {
   NODE *pHead;
   NODE *pTail;
   int iLength;
} LL_DOUBLE;

/* Creation functions */
LL_DOUBLE *CreateListDouble();
int DestroyListDouble(LL_DOUBLE **ppList);

/* Getter functions */
NODE *Get(LL_DOUBLE pList, int i);
void *GetData(LL_DOUBLE pList, int i);

/* List modification functions - return OK or ERROR */
int Push(LL_DOUBLE *pList, void *pvData);
int Append(LL_DOUBLE *pList, void *pvData);
int RemoveFromList(LL_DOUBLE *pList, NODE *pToDelete);

#endif /*ndef ___LL_DOUBLE_H___  */ 
