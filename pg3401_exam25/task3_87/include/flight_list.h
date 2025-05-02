/*--------------------------------------------------
 TODO: DOCUMENTATION
* AUTHOR: BKN
* DESCRIPTION: Doubly linked list implementation
*              which holds anonymous datatype in
*              form of a void *. Hence it is made
*              to be used as a baseline for further
*              abstractions.
--------------------------------------------------*/

/* Inclusion is in header so i dont have to redefine PASSENGER_LIST struct */
#include "passenger_list.h"

#ifndef ___FLIGHT_LIST_H___ 
#define ___FLIGHT_LIST_H___  

#define MAX_ID 5
#define MAX_DESTINATION 1028
#define MAX_SEATS 64

#pragma pack(1)

/*
 * PREFIX: fd - (f)light (d)ata
 * */
typedef struct _FLIGHT_DATA{
   char szID[5];
   char *pszDestination;
   int iDepartureTime;
   PASSENGER_LIST *pplPassengers;
} FLIGHT_DATA;

#pragma pack()

/*
 * PREFIX: f - (f)light
 * */
typedef struct _FLIGHT {
   struct _FLIGHT *pfNext;
   struct _FLIGHT *pfPrev;
   int iSize;
   FLIGHT_DATA *pfdData;
} FLIGHT;

/* 
 * PREFIX: fl - (f)light (l)ist
 * */
typedef struct _FLIGHT_LIST {
   FLIGHT *pfFirst;
   FLIGHT *pfLast;
   int iLength;
} FLIGHT_LIST;

/* Creation functions */
FLIGHT_LIST *CreateFlightList();
int DestroyFlightList(FLIGHT_LIST *ppfl);

/* Getter functions */
FLIGHT_DATA *GetFlightDataByPosition(FLIGHT_LIST fl, int n);

int _GetFlightNumberByDestination(FLIGHT_LIST fl, char szDestination[]);

/* List modification functions - return OK or ERROR */
int AddFlight(FLIGHT_LIST *pfl, char *szID, int iDepartureTime, char szDestination[]);

/* Removes a flight based on flight id */
int RemoveFlight(FLIGHT_LIST *pfl, char szID[]);

void PrintPassengers(FLIGHT_DATA *pfd);

/* Prints the flight list to the terminal */
void PrintFlightList(FLIGHT_LIST *pfl);

int AddPassengerToFlight();
int ChangePassengerSeat();

int InternalFlightListTest();

#endif /*ndef ___FLIGHT_LIST_H___  */ 
