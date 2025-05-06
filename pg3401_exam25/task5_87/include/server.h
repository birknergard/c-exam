#ifndef TASK5_SERVER_H
#define TASK5_SERVER_H

#define MAX_ID 64
#define MAX_READ 4096
#define IP_STRING_SIZE 16
#define SERVER_MSGSIZE 51

/*
* Function for creating SERVER REPLY struct for convenience.
 * Takes the SERVER PROTOCOL type struct as a void pointer, statuscode, and a message
 * Works identically to printf for the message
 * */
int CreateServerReply(void *vpStruct, char szStatusCode[], char szFormat[], ... );

/*
 * Verifies that the header is correct
 * */
int VerifyHeader(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER stHead, int iByteLimit);

/*
 * Calculates the raw ip address number as a long int. Returns -1 if invalid string 
 * */
long int ParseIPv4Address(char szIp[]);

void GetIPv4AddressAsString(char *szDestination, long int liIPv4Address);

/* Shorthand function for closing sockets, 
 * so i have one less line to type for every error scenario :)
 * */
void CloseSockets(int *sockServer, int *sockClient);

#endif /*ndef-> TASK5_SERVER_H*/
