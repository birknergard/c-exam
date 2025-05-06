#include "ewpdef.h"

#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h> /* For INADDR_ANY */
#include <time.h> /* For Time */

#define MAX_ID 128

int main(int iArgC, char **arrpszArgV){
	/* Declaring variables */
	int iBinded, iListened;
	int sockServerDescriptor = -1;
   int sockClientDescriptor = -1;
	struct sockaddr_in saServerAddress = {0};
	struct sockaddr_in saClientAddress = {0};

	int iReceived, iSent;
	int iNewAddressLength;
   int iPortNumber;
   int iAddress = 0x7F000001;

   char *szServerID = NULL;  
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT *ewaServerAccept = NULL;

   /* Parse program arguments */
   if(strcmp(arrpszArgV[1], "-port") == 0){
      iPortNumber = atoi(arrpszArgV[2]);
   }
   if(strcmp(arrpszArgV[3], "-id") == 0){
      szServerID = arrpszArgV[4];
   }

	/* Open network socket (TCP/IP Protocol), as a stream */
	sockServerDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(sockServerDescriptor < 0){
		printf("Error when opening socket - errcode: %d", errno);
		close(sockServerDescriptor);
		return -1;
	}

	/* Sets address type */
	saServerAddress.sin_family = AF_INET; 

	/* Converts port (int) into TCP/IP byte order (from little to big endian) */
	saServerAddress.sin_port = htons(iPortNumber); 

	/* The server listens to any available network interfaces (wifi etc.) on the computer */
	saServerAddress.sin_addr.s_addr = htonl(iAddress);  
			
	/* Bind socket to address */
	if((iBinded = bind(
		sockServerDescriptor,
		(struct sockaddr *) &saServerAddress,
		sizeof(saServerAddress)
	)) < 0){
		printf("Error with bind() - errcode %d", errno);
		close(sockServerDescriptor); sockServerDescriptor = -1;
		return -1;
	} 

   /* Server Accept protocol */
   ewaServerAccept = (struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT *) 
      malloc(sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));

   if(ewaServerAccept == NULL){
      close(sockServerDescriptor); sockServerDescriptor = -1;
      close(sockClientDescriptor); sockClientDescriptor = -1;

      return -1;
   }
   memset(ewaServerAccept, 0, sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));
   strncpy(ewaServerAccept->stHead.acMagicNumber, "EWP", 3);

   char szDataSizeBfr[5];
   snprintf(szDataSizeBfr, 5, "%02d", sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));
	szDataSizeBfr[4] = 'X';

   strncpy(ewaServerAccept->stHead.acDataSize, szDataSizeBfr, 4);
   strncpy(ewaServerAccept->stHead.acDelimeter, "|", 1);
   
   /* Input protocol body values */
   strcpy(ewaServerAccept->acStatusCode, "220");
   strcpy(ewaServerAccept->acHardSpace, " ");
   snprintf(ewaServerAccept->acFormattedString, 51, "%X %s %s - %s:%s:%s, %ld",
            iAddress,
            "SMTP", 
            szServerID, 
            "06",
            "05",
            "2025",
            (int)time(NULL)
   );
	ewaServerAccept->acFormattedString[strcspn(ewaServerAccept->acFormattedString, "\0")] = 0;
   strncpy(ewaServerAccept->acHardZero, "\0", 1);

   printf("RAW protocol: %s\n", (char *) ewaServerAccept);
   printf("Size of RAW serveraccept protocol: %d\n", sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));
   printf("Size of serveraccept protocol: %d\n", sizeof(*ewaServerAccept));

	/* Make server listen for input */
	iListened =	listen(sockServerDescriptor, 1);
	if(iListened < 0){
		printf("Listen failed - errcode: %d", errno);
		close(sockServerDescriptor); sockServerDescriptor = -1;
		return -1;
	}

	/* Initialize client socket address to zero */
	sockClientDescriptor = 0;
	iNewAddressLength = sizeof(saClientAddress); 

	/* Handle connection */
   sockClientDescriptor = accept(
      sockServerDescriptor,
      (struct sockaddr *) &saClientAddress,
      (socklen_t *) &iNewAddressLength
   );
   if(sockClientDescriptor < 0){
      printf("%s: Accept failed! errcode - %d\n", szServerID, errno);
      close(sockServerDescriptor); sockServerDescriptor = -1;
      close(sockClientDescriptor); sockClientDescriptor = -1;

      return -1;
   }
   
   /* REQUEST STRUCTS (Prefixed with ewa) */


	/* Send SERVER ACCEPT message */
	if(send(
		sockClientDescriptor,
      ewaServerAccept,
      64,
      0
	) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
	} 

   free(ewaServerAccept);

	/* Take request */
   /*
	if((iReceived = recv(sockClientDescriptor, //, MAX_PACKET, 0)) < 0){
		printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
		return -1;
	} 
   */


	/* Close the sockets, then assign a secure exit value */
	close(sockServerDescriptor); sockServerDescriptor = -1;
   close(sockClientDescriptor); sockClientDescriptor = -1;

   exit(1);
   return 1;
}


