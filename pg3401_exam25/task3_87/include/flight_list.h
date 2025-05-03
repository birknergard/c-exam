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
#define MAX_NAME 256

/*
 * Structure for holding passengers name and age.
 * The data structure only accounts for unique names,
 * therefore a passenger can have different age on different flights but still be the same person.
 * A fix for this is having Flight list keep a record of every unique passenger.
 *
 * PREFIX: p - (p)assenger
 * */
#pragma pack(1)
typedef struct _PASSENGER{
   char *pszName;
   int iAge;
} PASSENGER;
#pragma pack()

/*
 * PREFIX: pn - (p)assenger (n)ode
 * */
#pragma pack (1)
typedef struct _PASSENGER_NODE {
    struct _PASSENGER_NODE *ppnNext;
    int iSeatNumber;
    PASSENGER *ppPassenger; /*Holds a pointer to the data instead of holding the data itself*/
} PASSENGER_NODE;
#pragma pack ()

/*
 * PREFIX: pl - (p)assenger (l)ist
 * */
#pragma pack (1)
typedef struct _PASSENGER_LIST{
    PASSENGER_NODE *ppnHead;
    int iLength;
} PASSENGER_LIST;
#pragma pack ()


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
   FLIGHT_DATA *pfdData;
} FLIGHT;

/* 
 * PREFIX: fl - (f)light (l)ist
 * */
typedef struct _FLIGHT_LIST {
   FLIGHT *pfFirst;
   FLIGHT *pfLast;
   PASSENGER **ppUniquePassengers;
   int iUniquePassengers;
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
void GetPassengersFlights(FLIGHT_LIST *pfl, char szPassengerName[]);

int InternalFlightListTest();

#endif /*ndef ___FLIGHT_LIST_H___  */ 
