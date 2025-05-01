/*--------------------------------------------------
  TODO: DOCUMENTATION
* AUTHOR: 87
* DESCRIPTION: Header file for declaring various data belonging to the passenger list datastructure.  
*		The data structure is based on a (sorted) singly linked list implementation.
* PREFIX: pl: For (l)ist of (p)assengers
-------------------------------------------------- */
#ifndef ___PASSENGER_LIST_H__
#define ___PASSENGER_LIST_H__

/* The longest name (according to google) is 747. So this seems like a reasonable max :) */
#define MAX_NAME 1028

#pragma pack (1)
/*
 * PREFIX: pd - (p)assenger (d)ata
 * */
typedef struct _PASSENGER_DATA{
    int iSeatNumber;
    char szName[MAX_NAME];
    int iAge;
} PASSENGER_DATA;

/*
 * PREFIX: p - (p)assenger
 * */
typedef struct _PASSENGER {
    struct _PASSENGER *ppNext;
    PASSENGER_DATA *ppdData; /*Holds a pointer to the data instead of holding the data itself*/
} PASSENGER;
#pragma pack ()

#pragma pack (1)
/*
 * PREFIX: pl - (p)assenger (l)ist
 * */
typedef struct _PASSENGER_LIST{
    PASSENGER *ppFirst;
    int iLength;
} PASSENGER_LIST;
#pragma pack ()

/*
 * Creates a new passenger. For internal use.
 * */
static PASSENGER *_NewPassenger(PASSENGER_DATA *ppd);
static int _DestroyPassenger(PASSENGER *pp){
/*
 * Initializes an empty passenger list
 * */
PASSENGER_LIST *NewPassengerList();
static int DestroyPassengerList(PASSENGER_LIST **pppl){

/*
 *  This function pushes a passenger into the linked list, and makes it the new head of the list.
 * */
void AddPassenger(PASSENGER_LIST *ppl, PASSENGER_DATA pd);

/*
 *  This function retrieves the data of a node given its nth position (n) in the list.
 *  This data is (for now) retrieved in the form of a void pointer which needs to be
 *  explicitly cast and dereferenced manually. For example, given a linked list "L":
 *
 *  int data = *(int) Get(LINKEDLIST, INDEX);
 * */
PASSENGER_DATA *GetPassengerData(PASSENGER_LIST *ppl, int n);

/*
 * Retrieves a passenger by the PASSENGER struct. For internal use.
 * */
static PASSENGER *_GetPassenger(PASSENGER_LIST *ppl, int n);

/* 
 * Removes a passenger, given its nth position in the passenger list 
 * */
int RemovePassenger(PASSENGER_LIST *ppl, int iSeatNumber);  

void ChangePassengerSeat(PASSENGER_LIST *ppl, int iPreviousSeat, int iNewSeat);

#endif /*ndef ___PASSENGER_LIST_H___ */
