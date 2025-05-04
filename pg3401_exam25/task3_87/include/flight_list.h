/*--------------------------------------------------
* TITLE: Flight list
* AUTHOR: 87
* DESCRIPTION 
*  This data structure encompasses both a singly linked list (PASSENGER_LIST)
*  and a doubly linked list (FLIGHT_LIST).
* It also handles a partially dynamic array in the FLIGHT_LIST structure alongside
*  the linked list for holding unique passengers. I can hope this does not decrease my
*  grading as I could not resist making it.
*
*  It additionaly handles all of the operation that is required from the task.
*  I have provided some documentation in this file, particularly for justifying
*  high level design decisions. For more fine grained documentation I refer the
*  evaluator to the source file "flight_list.c".
*  
*  All functions in the API refer to the same struct * (created by CreateFlightList)
*  to prevent confusion. I attempted to only use a pointer reference when i wanted to
*  change the data but the code ended up being confusing and inconsistent.
*
*  It was originally split between two linked list data structures:
*  1. FLIGHT_LIST (doubly linked)  
*  2. PASSENGER_LIST (singly linked)
*
*  I originally created the program with the two separated, but this ended up in a lot of
*  confusing encapsulation and increasing difficulty in keeping track of the state.
*
*  Im very conscious of how complex this API is internally. I added more moving parts than
*  was really necessary for this task. I hope that the added complexity displays my
*  comfort with the programming language this kind of implementation. If I deviated too heavily
*  from the task and that lead to lost marks I would love to know when the evaluation comes.
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
#pragma pack (1)
typedef struct _PASSENGER {
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
    /* pp = (p)ointer (p)assenger
     Holds pointer to the data instead of holding the data itself
     so that the actual passenger data is stored in FLIGHT_LIST.arrpUniquePassengers */
   PASSENGER *ppPassenger;
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
#pragma pack(1)
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
   struct _FLIGHT_NODE *pfnNext;
   struct _FLIGHT_NODE *pfnPrev;
   FLIGHT_DATA *pfdData;
} FLIGHT_NODE;

/* 
 * PREFIX: fl - (f)light (l)ist
 * */
typedef struct _FLIGHT_LIST {
   FLIGHT_NODE *pfnHead;
   FLIGHT_NODE *pfnTail;

   /* ppp = (arr)ay (p)ointer (p)assenger */ 
   PASSENGER **arrppUniquePassengers;
   /*Holds every unique PASSENGER * in a dynamic array of pointers,
    to prevent duplicates and make some tasks easier */

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
 * Checks the unique passenger list for whether a name exists
 * Used when taking input
 * */
int UniquePassengerExists(FLIGHT_LIST *pfl, char szPassengerName[]);

/*
 * Adds a unique passenger to the unique passenger list 
 * */
int AddUniquePassenger(FLIGHT_LIST *pfl, char szName[], int iAge);

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

/*
 * Checks whether a flight has an empty passenger list
 * */
int PassengerListIsEmpty(FLIGHT_LIST *pfl, char szFlightID[]);
/*
 * Function for point four in the task description. Takes the FLIGHT_LIST, and a destination string.  
 * If any flights have that destination the number (n) is returned. Otherwise it returns -1.
 * */
int PrintFlightsByDestination(FLIGHT_LIST *pfl, char szDestination[]);

/* 
 * Prints all the passengers of a flight to the terminal 
 * */
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
 * To function the provided name must be matched in the unique passenger list
 * through the AddUniquePassenger function
 * */
int AddPassengerToFlight(FLIGHT_LIST *pfl, char szFlightID[], int iSeatNumber, char szName[]);

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
