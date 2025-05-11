/*
 * TITLE: TASK 3 MAIN
 * AUTHOR: 87
 * DESCRIPTION
 *
 * */
#include "main.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
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
	AddOption(pMenu, "Add a flight", OptOne);
	AddOption(pMenu, "Add passenger to flight", OptTwo);
	AddOption(pMenu, "Display flight", OptThree);
	AddOption(pMenu, "Find flight number by destination", OptFour);
	AddOption(pMenu, "Delete flight", OptFive);
	AddOption(pMenu, "Change passengers seat", OptSix);
	AddOption(pMenu, "Find which flights passenger is on", OptSeven);
	AddOption(pMenu, "View passengers multiple booked flights", OptEight);

	/* Removing flight with passengers*/
	/*
	bdebug("Removing flight with passengers");
	RemoveFlight(pflFlights, "HH11");

	*/
	iStatus = StartMenu(pMenu, "\nTASK 3 - FLIGHT CREATOR");
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
    char *pszID = NULL, *pszDestination = NULL, *pszDepartureTime = NULL;
    int iFlightAdded;
    int iInputValid;

    puts("CURRENT FLIGHT LIST\n");

    PrintFlightListSimple(pflFlightList);

    printf("%s\n\n", "________________________________________________________");

    pszID = (char *) malloc(MAX_INPUT);
    if(pszID == NULL){
	pflFlightList = NULL;
	return;
    }


    /* Ask for FLIGHT ID */
    iInputValid = GetInput(1, (char *[]) {
	"Enter a new flight ID. has to be exactly 4 characters",
	}, (char *) "S", pszID
    );
    if(iInputValid != 0){
	printf("Invalid input.\n");
	free(pszID);
	pszID = NULL;
	pflFlightList = NULL;
	return;
    } 

    /* Validates */
    if(isValidFlightID(pflFlightList, pszID) == 0){
	pszDestination = (char *) malloc(MAX_INPUT);
	if(pszDestination == NULL){
	    free(pszID);
	    pszID = NULL;
	    pflFlightList = NULL;
	    return;
	}

	pszDepartureTime = (char *) malloc(MAX_INPUT);
	if(pszDepartureTime == NULL){
	    free(pszID);
	    pszID = NULL;
	    free(pszDestination);
	    pszDestination = NULL;
	    pflFlightList = NULL;
	    return;
	}
	/* If that is validated, ask for remaining data */
	iInputValid = GetInput(2, (char *[]) {
	    "Enter the flights destination name:",
	    "Enter the flights departure time (\'HHMM\', example: 1705):",
	    }, (char *) "SS", pszDestination, pszDepartureTime 
	);
	if(iInputValid != 0){
	    printf("Invalid input.\n");
	    free(pszID);
	    pszID = NULL;
	    free(pszDestination);
	    pszDestination = NULL;
	    free(pszDepartureTime);
	    pszDepartureTime = NULL;
	    pflFlightList = NULL;
	    return;
	} 

	/* Do stuff */
	iFlightAdded = AddFlight(pflFlightList, pszID, pszDepartureTime, pszDestination);
	if(iFlightAdded == 0){
	    printf("Flight added on id %s!\n\n", pszID);

	} else printf("Could not add flight.\n\n");
    }

    /* Cleanup */
    free(pszID);
    free(pszDestination);
    free(pszDepartureTime);
    pszID = NULL;
    pszDestination = NULL;
    pszDepartureTime = NULL;

    pflFlightList = NULL;
    return;
}

/*
 * Option 2: Add passenger to flight
 * */
void OptTwo(void *vpflFlightList){
    /* Declaring variables */
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    char *pszFlightID = NULL, *pszPassengerName = NULL;
    int iSeatNumber = -1, iPassengerAge = -1;
    int iInputValid = 0;

    /* Print flight list */
    printf("Here are the current flights in the list\n");
    if(PrintFlightListSimple(pflFlightList) == 1){
	return;
    }

    pszFlightID = (char *) malloc(MAX_INPUT);
    if(pszFlightID == NULL){
	return;
    }

    pszPassengerName = (char *) malloc(MAX_INPUT);
    if(pszPassengerName == NULL){
	free(pszFlightID);
	pszFlightID = NULL;
	pszPassengerName = NULL;
	pflFlightList = NULL;
	return;
    }

    /* Ask for FLIGHT ID */
    iInputValid = GetInput(1, (char *[]) {
    "Enter registered >FLIGHT ID(XXXX)< (has to be exactly 4 characters):",
    }, (char *) "S", pszFlightID
    );
    if(iInputValid != 0){
	free(pszPassengerName);
	free(pszFlightID);
	pszFlightID = NULL;
	pszPassengerName = NULL;
	pflFlightList = NULL;
	return;
    }

    /* Validates */
    if(isValidFlightID(pflFlightList, pszFlightID) == 1){

	/* Promts for input */
	iInputValid = GetInput(1, (char *[]) {
	    "Enter passenger NAME:",
	}, (char *) "S", pszPassengerName);
	if(iInputValid != 0){
	    free(pszPassengerName);
	    free(pszFlightID);
	    pszFlightID = NULL;
	    pszPassengerName = NULL;
	    pflFlightList = NULL;
	    return;
	}

	/* Checks if passenger already exists, if yes */
	if(UniquePassengerExists(pflFlightList, pszPassengerName) == 0){
	    /* Otherwise prompt for age and add them */
	    printf("\nSeems this person is not in any other flights, so we need some more info.\n\n");
	    iInputValid = GetInput(1, (char *[]) {
		"Enter the passengers AGE:"
	    }, (char *) "I", &iPassengerAge);
	    if(iInputValid != 0){
		free(pszPassengerName);
		free(pszFlightID);
		pszFlightID = NULL;
		pszPassengerName = NULL;
		pflFlightList = NULL;
		return;
	    }

	    /* Add the passenger if they dont exist */
	    AddUniquePassenger(pflFlightList, pszPassengerName, iPassengerAge);
	}

	/* Ask for seat number */
	iInputValid = GetInput(1, (char *[]) {
	    "Assign a seat (0 -> 64):",
	}, (char *) "I", &iSeatNumber);
	if(iInputValid != 0){
	    free(pszPassengerName);
	    free(pszFlightID);
	    pszFlightID = NULL;
	    pszPassengerName = NULL;
	    pflFlightList = NULL;
	    return;
	}

	/* Add passenger to flight */
	AddPassengerToFlight(pflFlightList, pszFlightID, iSeatNumber, pszPassengerName);
    } else printf("Flight is not registered.\n");

    free(pszFlightID);
    free(pszPassengerName);
    pszFlightID = NULL;
    pszPassengerName = NULL;

    pflFlightList = NULL;
    return;
}

/*
 * Option 3: Retrieve flight N from the list and print all data associated
 * TODO: Make initial print not print flight ids like that for clarity
 * */
void OptThree(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    int iFlightNumber = -1;
    int iInputValid;

    printf("Here are the current flights in the list\n");
    if(PrintFlightListSimple(pflFlightList) == 1){
	pflFlightList = NULL;
	return;
    }
    puts("\n");

    iInputValid = GetInput(1, (char *[]) {
        "Enter Flight NUMBER:"
    }, (char *) "I", &iFlightNumber);
    if(iInputValid != 0){
	printf("Invalid input\n");
	return;
    }

    PrintFlight(pflFlightList, iFlightNumber);

    pflFlightList = NULL;
}

/*
 * Option 4: Find flight that matches destination, return item number
 * */
void OptFour(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;

    int iFlights;
    int iInputValid;
    char *pszDestination = NULL;

    if(pflFlightList->pfnHead == NULL){
	printf("\n\nThere are no flights in the list.\n");
	pflFlightList = NULL;
	return;
    }

    pszDestination = (char *) malloc(MAX_INPUT);
    if(pszDestination == NULL){
	pflFlightList = NULL;
	return;
    }

    iInputValid = GetInput(1, (char*[]) {
	"Enter Destination:"
    }, (char *) "S", pszDestination);
    if(iInputValid != 0){
	printf("Invalid input\n");
	free(pszDestination);
	pszDestination = NULL;
	pflFlightList = NULL;
	return;
    }

    iFlights = PrintFlightsByDestination(pflFlightList, pszDestination);
    if(iFlights > 0){
	printf("%d matches were found.\n", iFlights);
    } else {
	printf("Flight with matching destination could not be found.\n\n");
    }

    free(pszDestination);
    pszDestination = NULL;
    pflFlightList = NULL;
}

/*
 * Option 5: Delete a flight
 * */
void OptFive(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    char *pszFlightID = NULL;
    int iFlightDeleted;
    int iInputValid = 0;


    printf("Here are the current flights in the list\n");
    if(PrintFlightListSimple(pflFlightList) == 1){
	pflFlightList = NULL;
	return;
    }
    puts("\n");

    pszFlightID = (char *) malloc(MAX_INPUT);
    if(pszFlightID == NULL){
	pflFlightList = NULL;
	return;
    }

    iInputValid = GetInput(1, (char *[]) {
	"\nEnter flight ID to delete:"
	}, (char *) "S", pszFlightID);
    if(iInputValid != 0){
	printf("Invalid input\n");
	free(pszFlightID);
	pszFlightID = NULL;
	pflFlightList = NULL;
	return;
    }

    /* Validates */
    if(isValidFlightID(pflFlightList, pszFlightID) == 1){
	/* If that is validated, ask for remaining data */
	iFlightDeleted = RemoveFlight(pflFlightList, pszFlightID);
	if(iFlightDeleted == 0){
	    printf("Flight %s was deleted!\n\n", pszFlightID);
	} else {
	    printf("Flight could not be deleted.\n\n");
	}
    }

    free(pszFlightID);
    pszFlightID = NULL;
    pflFlightList = NULL;

}

/*
 * Option 6: Change passenger seat
 * */
void OptSix(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;

    char *pszFlightID = NULL, *pszName = NULL;
    int iNewSeat = -1;
    int iChangedSeat;
    int iInputValid = 0;

    pszFlightID = (char *) malloc(MAX_INPUT);
    if(pszFlightID == NULL){
	pflFlightList = NULL;
	return;
    }

    printf("Here are the current flights in the list\n");
    if(PrintFlightListSimple(pflFlightList) == 1){
	pflFlightList = NULL;
	return;
    }
    puts("\n");

    /* Get flight id first */
    iInputValid = GetInput(1, (char *[]) {
	"\nEnter Flight ID:"
	}, (char *) "S", pszFlightID);
    if(iInputValid != 0){
	printf("Invalid input\n");
	free(pszFlightID);
	pszFlightID = NULL;
	pflFlightList = NULL;
	return;
    }

    /* Validates */
    if(isValidFlightID(pflFlightList, pszFlightID) == 1){

	/* Show the list of passengers for reference */
	if(PassengerListIsEmpty(pflFlightList, pszFlightID) != 0){
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

	    iInputValid = GetInput(2, (char*[]) {
		"Enter passenger NAME:",
		"Choose the new SEAT:",
	    }, (char *) "SI", pszName, &iNewSeat);
	    if(iInputValid != 0){
		printf("Invalid input\n");
		free(pszName);
		pszName = NULL;
		free(pszFlightID);
		pszFlightID = NULL;
		pflFlightList = NULL;
		return;
	    }

	    iChangedSeat = ChangePassengerSeat(pflFlightList, pszFlightID, pszName, iNewSeat);
	    if(iChangedSeat == 0){
		printf("Successfully changed seat for %s on flight %s!\n\n", pszName, pszFlightID);
	    } else {
		printf("Failed to change seat for %s.\n\n", pszName);
	    }
	}
    }

    free(pszFlightID);
    free(pszName);
    pszFlightID = NULL;
    pszName = NULL;
    pflFlightList = NULL;
}

/*
 * Option 7: Find all flights passenger is reserved for (by name)
 * */
void OptSeven(void *vpflFlightList){
    FLIGHT_LIST *pflFlightList = (FLIGHT_LIST *) vpflFlightList;
    char *pszName = NULL;
    int iInputValid;

    if(pflFlightList->pfnHead == NULL){
	printf("\nThere are no flights in the list.\n");
	pflFlightList = NULL;
	return;
    }

    pszName = (char *) malloc(MAX_INPUT);
    if(pszName == NULL){
	pflFlightList = NULL;
	return;
    }

    iInputValid = GetInput(1, (char*[]) {
	"Enter Passenger Name:"
    }, (char *){"S"}, pszName);
    if(iInputValid != 0){
	printf("Invalid input\n");
	free(pszName);
	pflFlightList = NULL;
	pszName = NULL;
	return;
    }

    
    if(GetPassengersFlights(pflFlightList, pszName, 1) == 0){
	printf("Passenger %s has not booked any flights.\n", pszName);
    } else printf("Displaying flights booked by %s \n", pszName);

    free(pszName);
    pflFlightList = NULL;
    pszName = NULL;
    return;
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
	printf(" No valid passengers were found.\n");
    return;
}
