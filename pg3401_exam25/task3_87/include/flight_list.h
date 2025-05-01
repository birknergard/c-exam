/*--------------------------------------------------
 TODO: DOCUMENTATION
* AUTHOR: BKN
* DESCRIPTION: Doubly linked list implementation
*              which holds anonymous datatype in
*              form of a void *. Hence it is made
*              to be used as a baseline for further
*              abstractions.
--------------------------------------------------*/

#ifndef ___FLIGHT_LIST_H___ 
#define ___FLIGHT_LIST_H___  

#include "passenger_list.h"

#define ID_SIZE 4

#pragma pack(1)

/*
 * PREFIX: fd - (f)light (d)ata
 * */
typedef struct _FLIGHT_DATA{
   char szFlightID[ID_SIZE];
   int iSeats;
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
int DestroyFlightList(FLIGHT_LIST **ppfl);

/* Getter functions */
FLIGHT_DATA *GetFlightData(FLIGHT_LIST fl, int n);

/* List modification functions - return OK or ERROR */
int AddFlight(FLIGHT_LIST *pfl, FLIGHT_DATA fd);

/* Removes a flight based on flight id */
int RemoveFlight(FLIGHT_LIST *pfl, char szFlightId[]);

#endif /*ndef ___FLIGHT_LIST_H___  */ 
