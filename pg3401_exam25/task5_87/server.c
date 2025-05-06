#include "ewpdef.h"

#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <netinet/in.h> /* For INADDR_ANY */
#include <time.h> /* For Time */

#define MAX_ID 128

int CreateAcFormattedString(void *vpStruct, int size, const char szFormat[], ...);
int CreateHeader(void *vpStruct, size_t iDataSize);

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
   
   /* Create header for ServerAccept protocol */
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT ewaServerAccept = {0};
   CreateHeader(&ewaServerAccept.stHead, sizeof(ewaServerAccept));
   
   /* Input protocol body values */
   strcpy(ewaServerAccept.acStatusCode, "220");
   strcpy(ewaServerAccept.acHardSpace, " ");
   CreateAcFormattedString(&ewaServerAccept.acFormattedString, 51, "%X %s %s - %s:%s:%s, %ld",
            iAddress,
            "SMTP", 
            szServerID, 
            "06",
            "05",
            "2025",
            (int)time(NULL)
   );
   strcpy(ewaServerAccept.acHardZero, "\0");

   printf("Size of RAW serveraccept protocol: %d\n", sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));
   printf("Size of serveraccept protocol: %d\n", sizeof(ewaServerAccept));

	/* Send SERVER ACCEPT message */
	if(send(sockClientDescriptor, &ewaServerAccept, sizeof(ewaServerAccept), 0) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
	} 

	/* Take request from client */
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

int CreateHeader(void *vpStruct, size_t iDataSize){
   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *stHead = (struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *) vpStruct; 

   /* Add magic number */
   strncpy(stHead->acMagicNumber, "EWP", 3);

   /* Need snprint for converting sizeof data to ascii with padding, but it adds a null terminator to the string when used.
    * Therefore we first create the string we want and then copy everything but the null terminator to the struct 
    * We also pad it to four bytes of ASCII by adding leading zeroes */

   char szDataSizeBfr[5];
   snprintf(szDataSizeBfr, 5, "%04d", sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));
   /* Removing NULL terminator to be sure */
	/*szDataSizeBfr[4] = 'X'; */

   strncpy(stHead->acDataSize, szDataSizeBfr, 4);
   strncpy(stHead->acDelimeter, "|", 1);
}

/* Create a formatted string without zero terminator. Takes an anonymous struct and required size of string.
 * As well as format. Works similar to printf */
int CreateAcFormattedString(void *vpStruct, int size, const char szFormat[], ...){
	va_list vaArgs;
   /* Max string is 128 */
	char szOutput[128] = {0};

	va_start(vaArgs, szFormat);
   /* Adding extra space for zero terminator so it doesn't truncate any data */
	vsnprintf(szOutput, size + 1, szFormat, vaArgs);
	va_end(vaArgs);

   /* Removes zero terminator for good measure */
	szOutput[strcspn(szOutput, "\0")] = 0;

   /* Copies the given size argument x bytes of the created string into the struct. */
   strncpy(vpStruct, szOutput, size);

   return 0;
}



