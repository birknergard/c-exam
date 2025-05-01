#include "ll_single.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* TODO: store a bool with data indicating whether pData was malloced or not */
/* TODO: Implement type declaration for list with LIST.szType */

/*
 *	NODE is a generic struct which can hold a pointer to any data,
 *	as well as a pointer to the "next" node.
 * */
static NODE *_NewNode(void *p){
	NODE *nopNew; 

	nopNew = (NODE*) malloc(sizeof(NODE));

	nopNew->pData = p;
	nopNew->pNext = NULL;

	return nopNew;
}

/*
 *	LIST struct which can hold a pointer to any data,
 *	as well as a pointer to the "next" node.
 * */
LIST *NewList(){
	LIST *liNew;
	liNew = (LIST*) malloc(sizeof(LIST));

	liNew->iLength = 0;
	liNew->noHead = NULL;

	return liNew;
}

static int _FreeNode(NODE *no){

	bool bIsMalloced = 0;	

	if(no == NULL){
		puts("Cannot free null pointer.");	
		return 1;
	}

	if(bIsMalloced){
		free(no->pData);
	}

	free(no);
	return 0;
}

int FreeList(LIST *lip){
	/* for tracking current node */
	if(lip == NULL)	return 1;

	if(lip->noHead == NULL){
		free(lip);	
	}

	NODE *noCurrent, *noNext;
	int i;

	noCurrent = lip->noHead;

	while(noCurrent != NULL){
		noNext = noCurrent->pNext;
		_FreeNode(noCurrent);
		noCurrent = noNext;
	}

	free(lip);
}

void Push(LIST *lip, void *pData){
	NODE *newNode;		
	newNode = _NewNode(pData); 

	/* If list is empty, creates new node at head */
	if(lip->iLength == 0){
		lip->noHead = newNode;	
		lip->iLength++;
		return;
	}

	newNode->pNext = lip->noHead;
	lip->noHead = newNode;
	lip->iLength++;	
}

void Append(LIST *lip, void *pData){
	NODE *newNode, *noTail;
	newNode = _NewNode(pData);

	/* If list is empty, creates new node at head */
	if(lip->iLength == 0){
		lip->noHead = newNode;	
		lip->iLength++;
		return;
	}

	noTail = _GetNode(lip, lip->iLength - 1);
	noTail->pNext = newNode;
	lip->iLength++;
}

void Insert(LIST *lip, void *pData, int iIndex);

static NODE *_GetNode(LIST *lip, int iIndex){
	int i;
	NODE *nopCurrent;
	nopCurrent = lip->noHead;	

	for(i = 0; i < iIndex; i++){
		nopCurrent = nopCurrent->pNext;
	}

	return nopCurrent;
}

void *GetValue(LIST *lip, int iIndex){
	int i;
	NODE *nopCurrent;
	nopCurrent = lip->noHead;	

	for(i = 0; i < iIndex; i++){
		nopCurrent = nopCurrent->pNext;
	}

	return nopCurrent->pData;
}

int RemoveFirst(LIST *lip){
	NODE *noOldHead;

	/*TODO: Make list be emptyable, list can be initialized as empty but still take values.*/
	if(lip->iLength == 0){
		printf("Cant remove element on empty list.\n");
		return 1;
	}

	if(lip->iLength == 1){
		lip->noHead = NULL;	
		lip->iLength = 0;
		return 0;
	}

	/* Holds the current head */
	noOldHead = lip->noHead;

	/* Sets head adress to second node */
	lip->noHead = noOldHead->pNext;

	/* Frees the previous head adress */
	_FreeNode(noOldHead);

	/* Decreases list size */
	lip->iLength--;

	return 0;
}

int RemoveLast(LIST *lip){
	int i;
	NODE *nopOldTail, *nopNewTail;

	if(lip->iLength == 0)
		return 1;

	if(lip->iLength == 1){
		lip->noHead = NULL;
		lip->iLength = 0;
		return 0;
	} 

	nopNewTail = _GetNode(lip, lip->iLength - 2);
	nopOldTail = nopNewTail->pNext; 

	_FreeNode(nopOldTail);
	nopNewTail->pNext = NULL;	

	lip->iLength--;

	return 0;		
}

/* For internal testing */
int main(void){
	printf("DEBUG1: Declaring LIST\n");

	LIST *liStart;
	int **ip, i, a;

	a = 25;

	liStart = NewList();
	if(liStart == NULL){
		puts("DEBUG: liStart is NULL");
		return 1;
	}
		
	
	ip = (int**) malloc(sizeof(long*) * 100);
	for(i = 0; i < 100; i++){
		ip[i] = (int*) malloc(sizeof(int));
		*ip[i] = i; 
	}

	for(i = 0; i < 100; i++){
		Push(liStart, ip[i]);	
	}


	printf("\n\n Current list (%d):", liStart->iLength);
	for(i = 0; i < liStart->iLength; i++){
		printf(" %d", *(int*) GetValue(liStart, i));	
	}
	printf("\n");


	printf("\n\n Popping first two values.");
	RemoveFirst(liStart);
	RemoveFirst(liStart);

	printf("\n Popping last three values.");
	RemoveLast(liStart);
	RemoveLast(liStart);
	RemoveLast(liStart);

	printf("\n\n Current list:");
	for(i = 0; i < liStart->iLength; i++){
		printf(" %d", *(int*) GetValue(liStart, i));	
	}
	printf("\n");

	printf("\n Appending 25 to end of list.");
	Append(liStart, &a);

	printf("\n\n Current list:");
	for(i = 0; i < liStart->iLength; i++){
		printf(" %d", *(int*) GetValue(liStart, i));	
	}
	printf("\n");

	FreeList(liStart);

	for(i = 0; i < 100; i++)
		free(ip[i]);	
	free(ip);

	return 0;
}
