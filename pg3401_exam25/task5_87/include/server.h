#ifndef TASK5_SERVER_H
#define TASK5_SERVER_H

#define MAX_ID 64
#define MAX_READ 1028
#define MAX_FILE 9998
#define IP_STRING_SIZE 16
#define SERVER_MSGSIZE 51

#define MSG_ACCEPT "250"
#define MSG_READY "354"
#define MSG_ERROR "501"
#define MSG_EXIT "221"

/* Since the structs provided for use in the server are identical in bit structure, 
 * i decided to make my own version which I can use more liberally*/
typedef struct {
   char acMagicNumber[3]; /* = EWP */
   char acDataSize[4]; /* = sizeof(structure below), as ASCII, always base 10, ignore leading 0 */
   char acDelimeter[1]; /* = | */
} EWA_HEAD;

typedef struct {
   EWA_HEAD stHead; 
   char acStatusCode[3]; 
   char acHardSpace[1]; /* Set to a space (ascii 0x20) */
   char acFormattedString[51]; /* For writing messages */
   char acHardZero[1]; /* Set to binary 0, ensures the received data is a zero-terminated string */
} EWA_PROTOCOL;

/* 
 * Runs HELO protocol
 * */
int RunProtocolHELO(int *sockClient, char szClientID[], long int *liClientIP);

int RunProtocolMAILFROM(int *sockClient, char szMailFrom[]);

int RunProtocolRCPTTO(int *sockClient, char szMailTo[]);

int RunProtocolDATA(int *sockClient);
/* 
 * Shorthand function for handling server errors
 * */
int HandleServerError(int *sockClient, EWA_PROTOCOL *ewaStruct, char szCode[], char szResponse[]);

/*
* Function for creating SERVER REPLY struct for convenience.
 * Takes the SERVER PROTOCOL type struct as a void pointer, statuscode, and a message
 * Works identically to printf for the message
 * */
int CreateServerReply(EWA_PROTOCOL *ewaStruct, char szStatusCode[], char szFormat[], ... );

/*
 * Verifies that the header is correct
 * */
int VerifyHeader(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER stHead, int iByteLimit, int iPrint);

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
