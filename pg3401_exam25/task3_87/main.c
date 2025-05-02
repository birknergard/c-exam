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
    char *pszID = NULL, *pszDestination = NULL;
    int iDepartureTime = -1;

    pszID = (char *) malloc(MAX_INPUT);
    pszDestination = (char *) malloc(MAX_INPUT);
    if(pszID == NULL) 
	return;

    if(pszDestination == NULL){
	free(pszID);
	pszID = NULL;
	return;
    }
    GetInput(
        (char *[]) {
	"Enter a new Flight ID. Has to be exactly 4 characters:",
	"Enter the flights destination name:",
	"Enter the flights departure time (just numbers):"
	}, (char *) "SSI",
	&pszID, &pszDestination, &iDepartureTime 
    );

    /* Do stuff */
    

    

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
    char *pszFlightID = NULL, *pszPassengerName = NULL;
    int iSeatNumber = -1, iPassengerAge = -1;
    
    pszFlightID = (char *) malloc(MAX_INPUT);
    pszPassengerName = (char *) malloc(MAX_INPUT);

    if(pszFlightID != NULL && pszPassengerName != NULL){
        GetInput((char *[]) {
            "Enter Flight ID:",
            "Enter Passenger Name:",
            "Enter Seat Number:",
            "Enter Passenger Age:"
        }, (char *){"SSII"}, &pszFlightID, &pszPassengerName, &iSeatNumber, &iPassengerAge);

        bdebug("Flight ID: %sPassenger Name: %sSeat: %d, Age: %d\n", pszFlightID, pszPassengerName, iSeatNumber, iPassengerAge);
    }

    free(pszFlightID);
    free(pszPassengerName);
    pszFlightID = NULL;
    pszPassengerName = NULL;
}

/*
 * Option 3
 * */
void OptThree(void *vpflFlightList){

    int iFlightNumber = -1;

    GetInput((char *[]) {
        "Enter Flight Number:"
    }, (char *){"I"}, &iFlightNumber);

    bdebug("Flight Number: %d\n", iFlightNumber);
}

/*
 * Option 4: Find flight that matches destination, return item number
 * */
void OptFour(void *vpflFlightList){

    char *pszDestination = NULL;
    pszDestination = (char *) malloc(MAX_INPUT);

    if(pszDestination != NULL){
        GetInput((char*[]) {
            "Enter Destination:"
        }, (char *){"S"}, &pszDestination);

        bdebug("Destination: %s\n", pszDestination);
    }

    free(pszDestination);
    pszDestination = NULL;
}

/*
 * Option 5: Delete a flight
 * */
void OptFive(void *vpflFlightList){

    char *pszFlightID = NULL;
    pszFlightID = (char *) malloc(MAX_INPUT);

    if(pszFlightID != NULL){
        GetInput((char*[]) {
            "Enter Flight ID to delete:"
        }, (char *){"S"}, &pszFlightID);

        bdebug("Deleting Flight ID: %s\n", pszFlightID);
    }

    free(pszFlightID);
    pszFlightID = NULL;
}

/*
 * Option 6: Change passenger seat
 * */
void OptSix(void *vpflFlightList){
    /*
    char *pszFlightID, *szName; 
    int iNewSeat;
    */
    char *pszFlightID = NULL, *pszName = NULL;
    int iNewSeat = -1;

    pszFlightID = (char *) malloc(MAX_INPUT);
    pszName = (char *) malloc(MAX_INPUT);

    if(pszFlightID != NULL && pszName != NULL){
        GetInput((char*[]) {
            "Enter Flight ID:",
            "Enter Passenger Name:",
            "Enter New Seat Number:"
        }, (char *){"SSI"}, &pszFlightID, &pszName, &iNewSeat);

        bdebug("Flight ID: %sPassenger: %sNew Seat: %d\n", pszFlightID, pszName, iNewSeat);
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
        GetInput((char*[]) {
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


