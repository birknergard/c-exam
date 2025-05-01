#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "menu.h"
#include "flight_list.h"

int main(void){
    bdebug("Testing program!");
    
    MENU *pMenu = NULL;

    pMenu = CreateMenu();
	
    DestroyMenu(&pMenu);

    /* Testing passenger list datastruct */

    PASSENGER_LIST *passengerList = CreatePassengerList();
    if (passengerList == NULL) {
        bdebug("Error: Failed to create passenger list.\n");
        return 1;
    }
    bdebug("Created a new passenger list.\n");

    // Add passengers with different seat numbers to test ordering
    bdebug("\nAdding passengers with varying seat numbers...\n");
    
    // Add passenger with seat number 2
    if (AddPassenger(passengerList, 2, "John Doe", 30) == OK) {
        bdebug("Passenger John Doe added (Seat 2).\n");
    }

    // Add passenger with seat number 5
    if (AddPassenger(passengerList, 5, "Jane Smith", 25) == OK) {
        bdebug("Passenger Jane Smith added (Seat 5).\n");
    }

    // Add passenger with seat number 1 (this should be added at the beginning)
    if (AddPassenger(passengerList, 1, "Alice Johnson", 35) == OK) {
        bdebug("Passenger Alice Johnson added (Seat 1).\n");
    }

    // Add passenger with seat number 3 (this should go between 2 and 5)
    if (AddPassenger(passengerList, 3, "Bob Lee", 28) == OK) {
        bdebug("Passenger Bob Lee added (Seat 3).\n");
    }

    // Add passenger with seat number 4 (this should go between 3 and 5)
    if (AddPassenger(passengerList, 4, "Charlie Brown", 40) == OK) {
        bdebug("Passenger Charlie Brown added (Seat 4).\n");
    }

    // Print the list of passengers to check the order
    bdebug("\nPassenger List (after adding passengers with varying seat numbers):\n");
    PrintList(passengerList);

    // Verify the list is sorted by seat number
    bdebug("\nVerifying seat number order:\n");
    PASSENGER *ppCurrent = passengerList->ppFirst;
    int previousSeatNumber = 0;
    int isOrdered = 1;
    while (ppCurrent != NULL) {
        if (ppCurrent->ppdData->iSeatNumber < previousSeatNumber) {
            bdebug("Error: Seat numbers are not in order! Seat number %d follows seat number %d.\n",
                ppCurrent->ppdData->iSeatNumber, previousSeatNumber);
            isOrdered = 0;
            break;
        }
        previousSeatNumber = ppCurrent->ppdData->iSeatNumber;
        ppCurrent = ppCurrent->ppNext;
    }
    if (isOrdered) {
        bdebug("Seat numbers are correctly ordered.\n");
    }

    // Remove passengers and verify the list updates correctly
    bdebug("\nRemoving a few passengers...\n");
    RemovePassenger(passengerList, "John Doe");
    bdebug("Removed John Doe.\n");

    // Print the list after removal
    bdebug("\nPassenger List After Removing John Doe:\n");
    PrintList(passengerList);

    // Verify the seat numbers are still ordered after removal
    bdebug("\nVerifying seat number order after removal:\n");
    ppCurrent = passengerList->ppFirst;
    previousSeatNumber = 0;
    isOrdered = 1;
    while (ppCurrent != NULL) {
        if (ppCurrent->ppdData->iSeatNumber < previousSeatNumber) {
            bdebug("Error: Seat numbers are not in order! Seat number %d follows seat number %d.\n",
                ppCurrent->ppdData->iSeatNumber, previousSeatNumber);
            isOrdered = 0;
            break;
        }
        previousSeatNumber = ppCurrent->ppdData->iSeatNumber;
        ppCurrent = ppCurrent->ppNext;
    }
    if (isOrdered) {
        bdebug("Seat numbers are still correctly ordered after removal.\n");
    }

    // Destroy the passenger list
    bdebug("\nDestroying the passenger list...\n");
    DestroyPassengerList(passengerList);
    bdebug("Passenger list destroyed successfully.\n");

    return 0;
}
