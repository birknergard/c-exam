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

#define MAX_ID 5
#define MAX_DESTINATION 1028
#define MAX_SEATS 64

/* The longest name (according to google) is 747. So this seems like a reasonable max :) */
#define MAX_NAME 1028

typedef struct _PASSENGER_INFO{
    char *pszName;
    int iAge;
} PASSENGER_INFO;

#pragma pack (1)
/*
 * PREFIX: pd - (p)assenger (d)ata
 * */
typedef struct _PASSENGER_DATA{
    int iSeatNumber;
    char *pszName;
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
#pragma pack(1)

/*
 * PREFIX: fd - (f)light (d)ata
 * */
typedef struct _FLIGHT_DATA{
   char szID[MAX_ID];
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
FLIGHT *GetFlightByPosition(FLIGHT_LIST *fl, int n);

int GetFlightNumberByDestination(FLIGHT_LIST *pfl, char szDestination[]);

int FlightListIsEmpty(FLIGHT_LIST *pfl);

int FlightIsEmpty(FLIGHT_LIST *pfl, char szID[]);

/* List modification functions - return OK or ERROR */
int AddFlight(FLIGHT_LIST *pfl, char *szID, int iDepartureTime, char szDestination[]);

/* Removes a flight based on flight id */
int RemoveFlight(FLIGHT_LIST *pfl, char szID[]);

int PrintPassengers(FLIGHT_LIST *pfl, char szFlightID[]);

/* Prints the flight list to the terminal */
int PrintFlight(FLIGHT_LIST *pfl, int n);
void PrintFlightList(FLIGHT_LIST *pfl);
void PrintFlightListSimple(FLIGHT_LIST *pfl);

int AddPassengerToFlight(FLIGHT_LIST *pfl, char szFlightID[], int iSeatNumber, char szName[], int iAge);
int ChangePassengerSeat(FLIGHT_LIST *pfl, char szFlightID[], char szName[], int iNewSeat);
int GetPassengersFlights(FLIGHT_LIST *pfl, char szPassengerName[]);

int InternalFlightListTest();

#endif /*ndef ___FLIGHT_LIST_H___  */ 
