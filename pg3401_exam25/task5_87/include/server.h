#ifndef TASK5_SERVER_H
#define TASK5_SERVER_H

#define MAX_ID 64
#define IP_STRING_SIZE 16

/*
 * Verifies that the header is correct
 * */
int VerifyHeader(void *vpstHead, int iRequestSizeMax);

/* uses below function */
int ParseRequestSize(char szSize[], int iMax);

/*
 * Calculates the raw ip address number as a long int. Returns -1 if invalid string 
 * */
long int ParseIPv4Address(char szIp[]);

void GetIPv4AddressAsString(char *szDestination, long int liIPv4Address);

/* Shorthand function for closing sockets, 
 * so i have one less line to type for every error scenario :)
 * */
void CloseSockets(int *sockServer, int *sockClient);

/* Create a formatted string without zero terminator. Takes an anonymous struct and required size of string.
 * As well as format. Works similar to printf 
 * */
int CreateAcFormattedString(void *vpStruct, int size, const char szFormat[], ... );

/* Takes a struct * and attempts to store a ewa type header on the given address
 * */
int CreateHeader(void *vpStruct, int iDataSize);

#endif /*ndef-> TASK5_SERVER_H*/
