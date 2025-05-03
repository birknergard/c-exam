/*--------------------------------------------------
 TODO: DOCUMENTATION
* AUTHOR: BKN
* DESCRIPTION 
*  This data structure encompasses both a singly linked list (PASSENGER_LIST)
*  and a doubly linked list (FLIGHT_LIST).
*
*  It also handles a partially dynamic array in the FLIGHT_LIST structure alongside
*  the linked list for holding unique passengers. I can hope this does not decrease my
*  grading as I could not resist making it.
*
*  It additionaly handles all of the operation that is required from the task.
*  I have provided some documentation in this file, particularly for justifying
*  high level design decisions. For more fine grained documentation I refer the
*  evaluator to the source file "flight_list.c".
*  
--------------------------------------------------*/

#ifndef ___FLIGHT_LIST_H___ 
#define ___FLIGHT_LIST_H___  

#define MAX_ID 5
#define MAX_DESTINATION 1028
#define MAX_SEATS 64
#define MAX_NAME 256

/*
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
 * PREFIX: fn - (f)light (n)
 * */
typedef struct _FLIGHT_NODE {
   struct _FLIGHT *pfnNext;
   struct _FLIGHT *pfnPrev;
   FLIGHT_DATA *pfdData;
} FLIGHT_NODE ;

/* 
 * PREFIX: fl - (f)light (l)ist
 * */
typedef struct _FLIGHT_LIST {
   FLIGHT_NODE *pfnHead;
   FLIGHT_NODE *pfnTail;
   PASSENGER **ppUniquePassengers;
   int iUniquePassengers;
   int iLength;
} FLIGHT_LIST;

/*
 * Initializes the FLIGHT_LIST structure in the form of a unique pointer.
 * Internally it also creates a list of unique passenger pointers, which are what is added to 
 * the passenger lists later on.
 * */
FLIGHT_LIST *CreateFlightList();

/*
 * Destroys a flight list created with CreateFlightList
 * */
int DestroyFlightList(FLIGHT_LIST *ppfl);

/*
 * Function for point four in the task description. Takes the FLIGHT_LIST, and a destination string.  
 * If any flights have that destination the number (n) is returned. Otherwise it returns -1.
 * */
int GetFlightNumberByDestination(FLIGHT_LIST *pfl, char szDestination[]);

/*
 * Inserts a new flight at the head of the flight list 
 * Makes sure that no two flights have the same id, and that the ID is only 4 characters.
 * Also makes sure destination name string is within MAX_DESTINATION bounds.
 * */
int AddFlight(FLIGHT_LIST *pfl, char *szID, int iDepartureTime, char szDestination[]);

/*
 * Removes and deletes a flight from the flight list
 * Takes a flight ID as parameter and makes sure it exists.
 * */
int RemoveFlight(FLIGHT_LIST *pfl, char szID[]);

/* Prints all the passengers of a flight to the terminal */
int PrintPassengers(FLIGHT_LIST *pfl, char szFlightID[]);

/* 
 * Various functions that print lists to the terminal 
 * The simple variant prints just flight info, while the other ones
 * also print the passenger list for the given flights
 * */
int PrintFlight(FLIGHT_LIST *pfl, int n);
void PrintFlightList(FLIGHT_LIST *pfl);
void PrintFlightListSimple(FLIGHT_LIST *pfl);


/*
 * Adds a passenger to a flight (given its Flight ID). 
 * Makes sure the added passenger only occurs once per flight.
 * */
int AddPassengerToFlight(FLIGHT_LIST *pfl, char szFlightID[], int iSeatNumber, char szName[], int iAge);

/*
 * Changes the seat of a passenger. 
 * Accounts for changing seat to invalid seat (occupied or out of bounds).
 * */
int ChangePassengerSeat(FLIGHT_LIST *pfl, char szFlightID[], char szName[], int iNewSeat);

/*
 * Gets a list of every flight a passenger is booked to, returns the number of flights
 * */
int GetPassengersFlights(FLIGHT_LIST *pfl, char szPassengerName[]);

/*
 * Checks every flight and prints every passenger that is booked to more than one flight.
 * */
int PrintPassengersWithMultipleFlights(FLIGHT_LIST *pfl);

#endif /*ndef ___FLIGHT_LIST_H___  */ 
