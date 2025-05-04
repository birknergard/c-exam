#include "main.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "util.h"
#include "menu.h"
#include "flight_list.h"

int main(void){
    bdebug("Testing program!");
    
    /* Declare data structures */
    MENU *pMenu = NULL;
    FLIGHT_LIST *pflFlights = NULL;
    int iStatus = -1;

    /* Initialize data structures and verify*/
    pflFlights = CreateFlightList();
    if(pflFlights == NULL){
	return ERROR;
    }

    pMenu = CreateMenu((void*) pflFlights);
    if(pMenu == NULL){
	berror("Error with menu creation.\n");
	iStatus = ERROR;
    } else {
	/* for testing :) */
	AddOption(pMenu, "Add flight", OptOne);
	AddOption(pMenu, "Add passenger to flight", OptTwo);
	AddOption(pMenu, "Display flight", OptThree);
	AddOption(pMenu, "Find flight number by destination", OptFour);
	AddOption(pMenu, "Delete flight", OptFive);
	AddOption(pMenu, "Change passengers seat", OptSix);
	AddOption(pMenu, "Find which flights passenger is on", OptSeven);
	AddOption(pMenu, "View passengers multiple booked flights", OptEight);

	AddFlight(pflFlights, "HH11", 1234, "Korea");

	bdebug("Test->Adding duplicate flight ...\n");
	AddFlight(pflFlights, "HH11", 1234, "Korea");
	AddFlight(pflFlights, "HH11", 3265, "Norway");

	bdebug("Test->Adding some more flights\n");
	AddFlight(pflFlights, "KK11", 1234, "NOrway");
	AddFlight(pflFlights, "1123", 1234, "India");
	AddFlight(pflFlights, "KEK1", 1234, "Japan");

	/* Removing flight */
	bdebug("Test->Removing flight\n");
	RemoveFlight(pflFlights, "1123");
	/* Adding again */
	bdebug("Removing flight\n");
	AddFlight(pflFlights, "1123", 1234, "India");

	bdebug("Test->Adding passengers ...\n");
	AddUniquePassenger(pflFlights, "Marius", 26);
	AddUniquePassenger(pflFlights, "Marte", 20);
	AddUniquePassenger(pflFlights, "Bengt", 37);

	AddPassengerToFlight(pflFlights, "HH11", 26, "Marius");
	AddPassengerToFlight(pflFlights, "HH11", 1, "Marte");
	AddPassengerToFlight(pflFlights, "HH11", 5, "Bengt");

	bdebug("Attempt to add passenger not in unique flight list\n");
	AddPassengerToFlight(pflFlights, "HH11", 50, "NONAME");

	bdebug("Test->Adding duplicate passenger to HH11\n");
	AddPassengerToFlight(pflFlights, "HH11", 5, "Bengt");

	bdebug("Test->Adding known passenger to another flight\n");
	AddPassengerToFlight(pflFlights, "1123", 2, "Bengt");

	
	bdebug("Test-> Printing whole flight list");
	PrintFlightListSimple(pflFlights);

	bdebug("Test-> Change seat of passenger");
	ChangePassengerSeat(pflFlights, "HH11", "Marius", 3);

	bdebug("Test-> Change seat of passenger to one that is taken");
	ChangePassengerSeat(pflFlights, "HH11", "Marius", 26);
	ChangePassengerSeat(pflFlights, "HH11", "Marius", 5);

	bdebug("Test-> Change seat of passenger to one that is out of bounds");
	ChangePassengerSeat(pflFlights, "HH11", "Marius", 65);

	/* Removing flight with passengers*/
	bdebug("Removing flight with passengers");
	RemoveFlight(pflFlights, "HH11");

	iStatus = StartMenu(pMenu, "Task 3");
    }

    /* Destroy structs */	
    DestroyMenu(pMenu);
    DestroyFlightList(pflFlights);

    /* Remove danglers */
    pMenu = NULL;
    pflFlights = NULL;

    return iStatus;
}

/* 
 * Option 1: Add a flight to the list
 * */
void OptOne(void *vpflFlightList){
    /* Declaring variables */
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    char *pszID = NULL, *pszDestination = NULL;
    int iDepartureTime = -1;
    int iFlightAdded;

    puts("Current Flight List");
    PrintFlightListSimple(pflFlightList);

    pszID = (char *) malloc(MAX_INPUT);
    if(pszID == NULL) 
	return;

    pszDestination = (char *) malloc(MAX_INPUT);
    if(pszDestination == NULL){
	free(pszID);
	pszID = NULL;
	return;
    }

    /* Ask for FLIGHT ID */
    GetInput(1, (char *[]) {
	"Enter a new flight ID. has to be exactly 4 characters",
	}, (char *) "S", &pszID
    );

    /* Validates */
    if(isValidFlightID(pszID) == 0){
	/* If that is validated, ask for remaining data */
	GetInput(2, (char *[]) {
	    "Enter the flights destination name:",
	    "Enter the flights departure time (\'HHMM\', example: 1705):",
	    }, (char *) "SI", pszDestination, &iDepartureTime 
	);

	/* Do stuff */
	iFlightAdded = AddFlight(pflFlightList, pszID, iDepartureTime, pszDestination);
	if(iFlightAdded == OK){
	    printf("Flight added on id %s!\n\n", pszID);

	} else printf("Could not add flight.\n\n");
    }

    /* Cleanup */
    free(pszID);
    free(pszDestination);
    pszID = NULL;
    pszDestination = NULL;
}

/*
 * Option 2: Add passenger to flight
 * */
void OptTwo(void *vpflFlightList){
    /* Declaring variables */
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    char *pszFlightID = NULL, *pszPassengerName = NULL;
    int iSeatNumber = -1, iPassengerAge = -1;
    
    pszFlightID = (char *) malloc(MAX_INPUT);
    pszPassengerName = (char *) malloc(MAX_INPUT);

    if(pszFlightID == NULL){
	return;
    }

    if(pszPassengerName == NULL){
	free(pszFlightID);
	pszFlightID = NULL;
	return;
    }

    /* Print flight list */
    printf("Here are the current flights in the list\n");
    PrintFlightListSimple(pflFlightList);
    puts("\n\n");

    /* Ask for FLIGHT ID */
    GetInput(1, (char *[]) {
	"Enter a new flight ID. has to be exactly 4 characters",
	}, (char *) "S", &pszFlightID
    );

    /* Validates */
    if(isValidFlightID(pszFlightID) == 0){
	/* Promts for input */
	GetInput(1, (char *[]) {
	    "Enter passenger name:",
	}, (char *) "S", &pszPassengerName);

	/* Checks if passenger already exists, if yes */
	if(UniquePassengerExists(pflFlightList, pszPassengerName) != 0){
	    /* Otherwise prompt for age and add them */
	    printf("Seems this person is not in any other flights, so we need some more info.\n");
	    GetInput(1, (char *[]) {
		"Enter passenger age:"
	    }, (char *) "I", &iPassengerAge);

	    /* Add the passenger if they dont exist */
	    AddUniquePassenger(pflFlightList, pszPassengerName, iPassengerAge);
	}
    }
  
    /* Ask for seat number */
    GetInput(1, (char *[]) {
	"Assign a seat (0->64):",
    }, (char *) "I", &iSeatNumber);

    /* Add passenger to flight */
    AddPassengerToFlight(pflFlightList, pszFlightID, iSeatNumber, pszPassengerName);

    free(pszFlightID);
    free(pszPassengerName);
    pflFlightList = NULL;
    pszFlightID = NULL;
    pszPassengerName = NULL;
}

/*
 * Option 3: Retrieve flight N from the list and print all data associated
 * TODO: Make initial print not print flight ids like that for clarity
 * */
void OptThree(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    int iFlightNumber = -1;

    printf("Here are the current flights in the list\n");
    PrintFlightListSimple(pflFlightList);
    puts("\n");

    GetInput(1, (char *[]) {
        "Enter Flight NUMBER:"
    }, (char *) "I", &iFlightNumber);

    PrintFlight(pflFlightList, iFlightNumber);

    pflFlightList = NULL;
}

/*
 * Option 4: Find flight that matches destination, return item number
 * */
void OptFour(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;

    int iFlights;
    char *pszDestination = NULL;

    pszDestination = (char *) malloc(MAX_INPUT);
    if(pszDestination == NULL){
	pflFlightList = NULL;
	return;
    }

    GetInput(1, (char*[]) {
	"Enter Destination:"
    }, (char *) "S", &pszDestination);

    iFlights = PrintFlightsByDestination(pflFlightList, pszDestination);
    if(iFlights > 0){
	printf("%d matches were found.\n", iFlights);
    } else {
	printf("Flight with matching destination could not be found.\n\n");
    }

    free(pszDestination);
    pszDestination = NULL;
}

/*
 * Option 5: Delete a flight
 * */
void OptFive(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    char *pszFlightID = NULL;
    int iFlightDeleted;

    pszFlightID = (char *) malloc(MAX_INPUT);
    if(pszFlightID == NULL){
	pflFlightList = NULL;
	return;
    }

    printf("Here are the current flights in the list\n");
    PrintFlightListSimple(pflFlightList);
    puts("\n");

    GetInput(1, (char *[]) {
	"\nEnter flight ID to delete:"
	}, (char *) "S", &pszFlightID
    );

    /* Validates */
    if(isValidFlightID(pszFlightID) == 0){
	/* If that is validated, ask for remaining data */
	bdebug("Deleting Flight ID: %s\n", pszFlightID);
	iFlightDeleted = RemoveFlight(pflFlightList, pszFlightID);
	if(iFlightDeleted == 0){
	    printf("Flight with id %s was deleted!\n\n", pszFlightID);
	} else {
	    printf("Flight could not be deleted.\n\n");
	}
    }

    free(pszFlightID);
    pszFlightID = NULL;
}

/*
 * Option 6: Change passenger seat
 * */
void OptSix(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;

    char *pszFlightID = NULL, *pszName = NULL;
    int iNewSeat = -1;
    int iChangedSeat;

    pszFlightID = (char *) malloc(MAX_INPUT);
    if(pszFlightID == NULL){
	pflFlightList = NULL;
	return;
    }

    printf("Here are the current flights in the list\n");
    PrintFlightListSimple(pflFlightList);
    puts("\n");

    /* Get flight id first */
    GetInput(1, (char *[]) {
	"\nEnter flight ID to delete:"
	}, (char *) "S", &pszFlightID
    );

    /* Validates */
    if(isValidFlightID(pszFlightID) == 0){

	/* Show the list of passengers for reference */
	if(PassengerListIsEmpty(pflFlightList, pszFlightID)){
	    printf("Could not continue with action.\n");
	} else {
	    /* Take more input */
	    PrintPassengers(pflFlightList, pszFlightID);

	    pszName = (char *) malloc(MAX_INPUT);
	    if(pszName == NULL){
		free(pszFlightID);
		pszFlightID = NULL;
		pflFlightList = NULL;
		return;
	    }

	    GetInput(2, (char*[]) {
		"Enter passenger name:",
		"Enter NEW seat number (0->64):"
	    }, (char *) "SI", &pszName, &iNewSeat);

	    iChangedSeat = ChangePassengerSeat(pflFlightList, pszFlightID, pszName, iNewSeat);
	    if(iChangedSeat == 0){
		printf("Successfully changed seat for %s on flight #%s!\n\n", pszName, pszFlightID);
	    } else {
		printf("Failed to change seat for %s.\n\n", pszName);
	    }
	}
    }


    free(pszFlightID);
    free(pszName);
    pszFlightID = NULL;
    pszName = NULL;
}

/*
 * Option 7: Find all flights passenger is reserved for (by name)
 * */
void OptSeven(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    char *pszName = NULL;

    pszName = (char *) malloc(MAX_INPUT);
    if(pszName == NULL){
	pflFlightList = NULL;
	return;
    }

    GetInput(1, (char*[]) {
	"Enter Passenger Name:"
    }, (char *){"S"}, &pszName);

    printf("Displaying flights booked by %s \n", pszName);
    if(GetPassengersFlights(pflFlightList, pszName) == 0){
	printf("Passenger %s has not booked any flights.\n", pszName);
    };

    free(pszName);
    pflFlightList = NULL;
    pszName = NULL;
}

/*
 * Option 8: Find passengers which are booked for more than one flight
 * */
void OptEight(void *vpflFlightList){
    /* No input needed :) */
    int iPassengers;
    printf("Looking for passengers on multiple flights...\n");

    iPassengers = PrintPassengersWithMultipleFlights((FLIGHT_LIST *) vpflFlightList);
    if(iPassengers == 1)
	printf("No valid passengers were found.\n");
}
