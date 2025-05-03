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
    int iStatus;

    /* Initialize data structures and verify*/
    pflFlights = CreateFlightList();
    if(pflFlights == NULL){
	return ERROR;
    }

    pMenu = CreateMenu((void*) pflFlights);
    if(pMenu == NULL){
	iStatus = ERROR;
    } else {
	AddOption(pMenu, "Add flight", OptOne);
	AddOption(pMenu, "Add passenger to flight", OptTwo);
	AddOption(pMenu, "Display flight", OptThree);
	AddOption(pMenu, "Find flight number by destination", OptFour);
	AddOption(pMenu, "Delete flight", OptFive);
	AddOption(pMenu, "Change passengers seat", OptSix);
	AddOption(pMenu, "Find which flights passenger is on", OptSeven);
	AddOption(pMenu, "View passengers multiple booked flights", OptEight);

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

/* TODO:
 * Step 1: Figure out what variables each option need from the user 
 * Step 2: Handle input from user (Make a function which takes variable inputs, for just int and string)
 * Step 3: Implement function
 * Step 4: Test thoroughly
 * Step 5: Complete documentation for everything
 *
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

    GetInput(3, (char *[]) {
	"Enter a new flight ID. Has to be exactly 4 characters:",
	"Enter the flights destination name:",
	"Enter the flights departure time (\'HHMM\', example: 1705):",
	}, (char *) "SSI", &pszID, &pszDestination, &iDepartureTime 
    );

    /* Do stuff */
    iFlightAdded = AddFlight(pflFlightList, pszID, iDepartureTime, pszDestination);
    if(iFlightAdded == OK){
	printf("Flight added on id %s!\n\n", pszID);

    } else printf("Could not add flight.\n\n");

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
    puts("\n");

    GetInput(4, (char *[]) {
	"Enter flight ID (4 characters/numbers):",
	"Enter passenger name:",
	"Enter seat number:",
	"Enter passenger age:"
    }, (char *) "SSII", &pszFlightID, &pszPassengerName, &iSeatNumber, &iPassengerAge);


    /* If not, allow to add passenger on id*/
    AddPassengerToFlight(pflFlightList, pszFlightID, iSeatNumber, pszPassengerName, iPassengerAge);

    free(pszFlightID);
    free(pszPassengerName);
    pflFlightList = NULL;
    pszFlightID = NULL;
    pszPassengerName = NULL;
}

/*
 * Option 3: Retrieve flight N from the list and print all data associated
 * */
void OptThree(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    int iFlightNumber = -1;

    printf("Here are the current flights in the list\n");
    PrintFlightListSimple(pflFlightList);
    puts("\n");

    GetInput(1, (char *[]) {
        "Enter Flight Number:"
    }, (char *) "I", &iFlightNumber);

    PrintFlight(pflFlightList, iFlightNumber);

    pflFlightList = NULL;
}

/*
 * Option 4: Find flight that matches destination, return item number
 *
 * PS: I have not controlled for destination being unique nor printing every flight
 *     a given destination (TODO:?), so the function will only print the first match
 * */
void OptFour(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;

    int iFlightNumber;
    char *pszDestination = NULL;

    pszDestination = (char *) malloc(MAX_INPUT);
    if(pszDestination == NULL){
	pflFlightList = NULL;
	return;
    }

    GetInput(1, (char*[]) {
	"Enter Destination:"
    }, (char *) "S", &pszDestination);

    iFlightNumber = GetFlightNumberByDestination(pflFlightList, pszDestination);
    if(iFlightNumber > 0){
	PrintFlight(pflFlightList, iFlightNumber);
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

    GetInput(1, (char*[]) {
	"\nEnter flight ID to delete:"
    }, (char *) "S", &pszFlightID);

    bdebug("Deleting Flight ID: %s\n", pszFlightID);
    iFlightDeleted = RemoveFlight(pflFlightList, pszFlightID);
    if(iFlightDeleted == 0){
	printf("Flight with id %s was deleted!\n\n", pszFlightID);
    } else printf("Flight could not be deleted.\n\n");

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

    pszName = (char *) malloc(MAX_INPUT);
    if(pszName == NULL){
	free(pszFlightID);
	pszFlightID = NULL;
	pflFlightList = NULL;
	return;
    }

    /* Get flight id first */
    GetInput(1, (char*[]) {
	"Enter flight ID:",
    }, (char *) "S", &pszFlightID);

    /* Show the list of passengers for reference */
    if(PrintPassengers(pflFlightList, pszFlightID) != 0){
	printf("Could not continue with action.\n");
    } else {
	/* Take more input */
	GetInput(2, (char*[]) {
	    "Enter passenger name:",
	    "Enter NEW seat number (0->64):"
	}, (char *) "SI", &pszName, &iNewSeat);

	iChangedSeat = ChangePassengerSeat(pflFlightList, pszFlightID, pszName, iNewSeat);
	if(iChangedSeat == 0){
	    printf("Successfully changed seat for %s on flight #%s!\n\n", pszName, pszFlightID);
	} else printf("Failed to change seat for %s.\n", pszName);
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
    /*
    char *pszName; 
    */
    char *pszName = NULL;
    pszName = (char *) malloc(MAX_INPUT);

    if(pszName != NULL){
        GetInput(1, (char*[]) {
            "Enter Passenger Name:"
        }, (char *){"S"}, &pszName);

        bdebug("Searching flights for: %s\n", pszName);
    }

    free(pszName);
}

/*
 * Option 8: Find passengers which are booked for more than one flight
 * */
void OptEight(void *vpflFlightList){
    /* No input needed :) */
    printf("Looking for passengers on multiple flights...\n");
}


