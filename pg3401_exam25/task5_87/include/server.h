#ifndef TASK5_SERVER_H
#define TASK5_SERVER_H

#define MAX_ID 64
#define MAX_READ 1028
#define MAX_FILE 9998
#define IP_STRING_SIZE 16
#define SERVER_MSGSIZE 51


/* Since the structs provided for use in the server are identical in bit structure, 
 * i decided to make my own version which I can use more liberally*/
typedef struct {
   char acMagicNumber[3]; /* = EWP */
   char acDataSize[4]; /* = sizeof(structure below), as ASCII, always base 10, ignore leading 0 */
   char acDelimeter[1]; /* = | */
} EWA_HEAD;

typedef struct {
   EWA_HEAD stHead; 
   char acCommand[4]; /* Set to QUIT */
   char acFormattedString[51]; /* Reserved for future use */
   char acHardZero[1]; /* Set to binary 0, ensures the received data is a zero-terminated string */
} EWP_PROTOCOL
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
