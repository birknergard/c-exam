/*--------------------------------------------------
 TODO: DOCUMENTATION
* AUTHOR: BKN
* DESCRIPTION: Doubly linked list implementation
*              which holds anonymous datatype in
*              form of a void *. Hence it is made
*              to be used as a baseline for further
*              abstractions.
--------------------------------------------------*/
#include "passenger_list.h"

#ifndef ___FLIGHT_LIST_H___ 
#define ___FLIGHT_LIST_H___  

#define ID_SIZE 4

#pragma pack(1)

/*
 * PREFIX: fd - (f)light (d)ata
 * */
typedef struct _FLIGHT_DATA{
   char szFlightID[ID_SIZE]
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
   FLIGHT *pfHead;
   FLIGHT *pfTail;
   int iFlights;
} FlIGHT_LIST;

static FLIGHT *CreateFlight(const FLIGHT_DATA *pfd);
/* Creation functions */
FLIGHT_LIST *CreatePlaneList();
int DestroyFlightList(FLIGHT_LIST **ppfl);

/* Getter functions */
FLIGHT *GetFlight(FLIGHT_LIST fl, int n);
FLIGHT_DATA *GetFlightData(FLIGHT_LIST fl, int n);

/* List modification functions - return OK or ERROR */
int AddFlight(FLIGHT_LIST *pfl, FLIGHT_DATA fd);

/* Removes a flight based on flight id */
int RemoveFlight(FLIGHT_LIST *pfl, char szFlightId);

#endif /*ndef ___LL_DOUBLE_H___  */ 
