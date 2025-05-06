/*
 * TITLE: Task 5 Server
 * AUTHOR: 87
 * DESCRIPTION
 *
 * TODO: Logging for each error
 * TODO: Make rough complete smtp transaction, then implement proper validation
 * */
#include "ewpdef.h"
#include "server.h"

#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include <netinet/in.h> /* For INADDR_ANY */
#include <time.h> /* For Time */


int main(int iArgC, char **arrpszArgV){
	/* Declaring server variables */
	int iBinded, iListened;
	int sockServer = -1;
   int sockClient = -1;
	struct sockaddr_in saServerAddress = {0};
	struct sockaddr_in saClientAddress = {0};
	int iNewAddressLength;
   int iPortNumber = -1;

   /* Sets address to localhost */
   long int iServerAddress = 0x7F000001;

   char *szServerID = NULL;  

   /* Client Info */
   long int liClientIP;
   char szClientIP[IP_STRING_SIZE];
   char szClientID[MAX_ID];

   /* Declaring iterator */
   int i;

   if(iArgC == 0){
      perror("Server attempted start without arguments.");
      return -1;
   }

   /* Parse program arguments */
   if(strcmp(arrpszArgV[1], "-port") == 0){
      iPortNumber = atoi(arrpszArgV[2]);
   }

   /* If flag was given, store id from arguments */
   if(strcmp(arrpszArgV[3], "-id") == 0){
      szServerID = arrpszArgV[4];
   }

	/* Open network socket (TCP/IP Protocol), as a stream */
	sockServer = socket(AF_INET, SOCK_STREAM, 0);
	if(sockServer < 0){
		printf("Error when opening socket - errcode: %d", errno);
		close(sockServer);
		return -1;
	}

	/* Sets address type */
	saServerAddress.sin_family = AF_INET; 

	/* Converts port (int) into TCP/IP byte order (from little to big endian) */
	saServerAddress.sin_port = htons(iPortNumber); 

	/* The server listens to any available network interfaces (wifi etc.) on the computer */
	saServerAddress.sin_addr.s_addr = htonl(iServerAddress);  
			
	/* Bind socket to address */
	if((iBinded = bind(
		sockServer,
		(struct sockaddr *) &saServerAddress,
		sizeof(saServerAddress)
	)) < 0){
		printf("Error with bind() - errcode %d", errno);
		close(sockServer); sockServer = -1;
		return -1;
	} 

	/* Make server listen for input */
	iListened =	listen(sockServer, 1);
	if(iListened < 0){
		printf("Listen failed - errcode: %d", errno);
		close(sockServer); sockServer = -1;
		return -1;
	}

	/* Initialize client socket address to zero */
	sockClient = 0;
	iNewAddressLength = sizeof(saClientAddress); 

	/* Handle connection */
   if((sockClient = accept(
      sockServer,
      (struct sockaddr *) &saClientAddress,
      (socklen_t *) &iNewAddressLength
   )) < 0){
      printf("%s: Accept failed! errcode - %d\n", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
      return -1;
   }
   
   /* Create header for ServerAccept protocol */
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT ewaServerAccept = {0};
   CreateHeader(&ewaServerAccept.stHead, sizeof(ewaServerAccept));
   
   /* Input protocol body values */
   strcpy(ewaServerAccept.acStatusCode, "220");
   strcpy(ewaServerAccept.acHardSpace, " ");
   CreateAcFormattedString(&ewaServerAccept.acFormattedString, 51, "%X %s %s - %s:%s:%s, %ld",
            iServerAddress,
            "SMTP", 
            szServerID, 
            "06",
            "05",
            "2025",
            (int)time(NULL)
   );
   strcpy(ewaServerAccept.acHardZero, "\0");

	/* Send SERVER ACCEPT */
	if(send(sockClient, &ewaServerAccept, sizeof(ewaServerAccept), 0) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
	}  

   /* Create struct for holding reply */
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO ewaClientHelo = {0};

   /* Receive data */
	if(recv(sockClient, &ewaClientHelo, sizeof(ewaClientHelo), 0) < 0){
		printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
		return -1;
	} 

   /* Create SERVERHELO protocol */
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERHELO ewaServerHelo = {0};
   CreateHeader(&ewaServerHelo.stHead, sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO));

   /* Verify data received is correct */
   if(
      /* Verify header */
      VerifyHeader(&ewaClientHelo.stHead, sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO)) == 0 &&

      /* Verifying body structure */
      (strncmp(ewaClientHelo.acCommand, "HELO", 4) == 0) &&
      ewaClientHelo.acHardSpace[0] == 0x20 && /* ASCI value for space */
      sizeof(ewaClientHelo.acFormattedString) <= 50 && sizeof(ewaClientHelo.acFormattedString) > 0 &&
      ewaClientHelo.acHardZero[0] == '\0' /* Zero terminator */
   ){
      /* Initialize local variables for client data */
      liClientIP = 0;
      memset(szClientID, 0, MAX_ID);

      /* Attempt to parse username into local buffer */
      for(i = 0; i < 42; i++){
         if(ewaClientHelo.acFormattedString[i] == '.'){
            szClientID[i] = '\0';
            break;
         }
         szClientID[i] = ewaClientHelo.acFormattedString[i]; 
      }

      /* ID needs to be at least two characters, 
       * max 42 (given that we need address as well, which is minimum 8 (1.1.1 */
      if(strlen(szClientID) >= 2 && strlen(szClientID) < 40){

         /* Check if remaining string is long enough to hold an IP address with . separators */
         if((strlen(ewaClientHelo.acFormattedString) - strlen(szClientID)) > 7){
            /* Parsing IP address. Starts calculating from where the id string ended (skipping the . terminator)*/
            liClientIP = ParseIPv4Address(ewaClientHelo.acFormattedString + (i + 1));

            GetIPv4AddressAsString(szClientIP, liClientIP);

            /* Verify IP address */
            if(liClientIP > 0){
               /* If all of these conditions are met, set serreply to OK */
               strcpy(ewaServerHelo.acStatusCode, "250" /*EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_OK*/);
               CreateAcFormattedString(&ewaServerHelo.acFormattedString, 51, "%d HELLO %s", szClientIP, szClientID);
               printf("SUCCESS: CLIENT ID=%s, IP=%s(%X)", szClientID, szClientIP, liClientIP);

            } else {
               perror("INVALID IP ADDRESS: ");
               printf("%ld\n", liClientIP);
               strcpy(ewaServerHelo.acStatusCode, "501");
            } 

         } else {
            perror("NO IP ADDRESS PROVIDED\n");
            strcpy(ewaServerHelo.acStatusCode, "501");
         }

      } else {
         perror("CLIENT ID IS INVALID\n");
         strcpy(ewaServerHelo.acStatusCode, "501");
      } 

   } else {
      perror("INVALID REQUEST STRUCTURE\n");
      CreateAcFormattedString(&ewaServerHelo.acFormattedString, 51, "BAD REQUEST");
      strcpy(ewaServerHelo.acStatusCode, "501");
   } 


   strcpy(ewaServerHelo.acHardZero, "\0");

   /* Send SERVERHELO to client */
	if(send(sockClient, &ewaServerHelo, sizeof(ewaServerHelo), 0) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
	}  


   CloseSockets(&sockServer, &sockClient);
   exit(1);
   return 1;
}

void GetIPv4AddressAsString(char *szDestination, long int liIPv4Address){
   char szBuffer[5];
   char szConverted[16];
   signed char byShiftedAddr;
   int i, j;


   memset(szDestination, 0, IP_STRING_SIZE);

   for(i = 0; i <= 24; i+=8){
      byShiftedAddr = liIPv4Address >> i;

      /* Resetting buffer */
      memset(szBuffer, 0, 4);

      /* Creating string */
      snprintf(szBuffer, 4, "%d.", byShiftedAddr);

      printf("IPSTRING=%s\n", szBuffer);

      /* Concatenating buffer to converted string */
      strncat(szConverted, szBuffer, 4);  
   }

   strncpy(szDestination, (char*) szConverted, IP_STRING_SIZE);
   szDestination[IP_STRING_SIZE] = '\0';
}

/* NOTE: I didn't test sending a raw ip address string at first so i thought this was required,
 * leaving it in anyway. I figure it's a nice way to verify the address is correct anyway. */
long int ParseIPv4Address(char szIp[]){
   /* Declaring variables */
   long int liIPv4 = 0;
   int iCurrentBitField = 4;
   int i, j = 0;
   unsigned char cDigit;
   char szBuffer[4];
   memset(szBuffer, 0, 4);

   printf("Address parser: Input IP string=%s\n", szIp);
   /* Calculates the raw ; i++ip address using shifting. 
    * Loops 15 times since 255.255.255.255 is 15 characters */ 
   for(i = 0; i < 15; i++){
      cDigit = szIp[i];

      /* Checking for valid characters */
      if(isdigit(cDigit) != 0 || cDigit == '.' || cDigit == '\0'){

         /* If string ends, or we have calculated all the bit fields, exit */
         if(iCurrentBitField == 0){
            break;
         }
         
         /* Completes calculation when it hits a period or zero terminator */
         if(cDigit == '.' || cDigit == '\0'){
            /* zero terminate before attempting int conversion */
            szBuffer[3] = '\0';
            printf("String segment for bit field %d=%s - i=%d, j=%d\n", iCurrentBitField, szBuffer, i, j);
            switch(iCurrentBitField){
               case 4:
                  liIPv4 += (atoi(szBuffer) << 24); 
                  memset(szBuffer, 0, 4);
                  break;

               case 3:
                  liIPv4 += (atoi(szBuffer) << 16); 
                  memset(szBuffer, 0, 4);
                  break;

               case 2:
                  liIPv4 += (atoi(szBuffer) << 8); 
                  memset(szBuffer, 0, 4);
                  break;

               case 1:
                  liIPv4 += atoi(szBuffer); 
                  memset(szBuffer, 0, 4);
                  break;
            }

            /* Checks next bit field */
            iCurrentBitField--;

            /* Resets the other iterator */
            j = 0;

         } else {
            /* If no break conditions are hit, keep copying the string into the buffer */
            szBuffer[j] = cDigit;
            j++;
         }
      } 
   }

   return liIPv4;
}

/*
 * Checks if request size given in header is a correct int, and if its the right value
 * */
int ParseRequestSize(char szSize[], int iMax){
   int iSize = 0, i;
   unsigned char cChar;

   if(strlen(szSize) != 4) return 1;

   for(i = 4; i > 0; i--){
      cChar = szSize[i];
      if(isdigit(szSize[i])){
         /* Numbers start at spot 48 in the ascii table (0), 
          * so we directly convert to int by subtracting 48 */
         cChar = cChar - 48;

         /* There has to be data in the request, so size cant be 0 */
         if(cChar == 0) return -1;

         /* if single digit, add as normal */
         if(i == 4) {
            iSize += cChar;
         /* Else, add a zero first */
         } else {
            /* Add the 10^n where n is the index, (lower index is higher n) */
            iSize += pow((double) 10, (double) atoi(szSize));
         }
      } else return -1;
   }

   /* Check if value exceeds max */
   if(iSize > iMax) return -1;

   return iSize;
}

int VerifyHeader(void *vpstHead, int iRequestSizeMax){
   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER ewaHead = *(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *) vpstHead;
   /* Makes sure request size is correct */ 
   if(
      strncmp(ewaHead.acMagicNumber, "EWP", 3) == 0 &&
      ParseRequestSize(ewaHead.acDataSize, iRequestSizeMax) > 0 &&
      ewaHead.acDelimeter[0] == '|'
   ) return 0;
   else return 1;

}

int CreateHeader(void *vpStruct, int iDataSize){
   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *stHead = (struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *) vpStruct; 

   /* Add magic number */
   strncpy(stHead->acMagicNumber, EWA_EXAM25_TASK5_PROTOCOL_MAGIC, 3);

   /* Need snprint for converting sizeof data to ascii with padding, but it adds a null terminator to the string when used.
    * Therefore we first create the string we want and then copy everything but the null terminator to the struct 
    * We also pad it to four bytes of ASCII by adding leading zeroes */

   char szDataSizeBfr[5];
   snprintf(szDataSizeBfr, 5, "%04d", sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT));
   /* Removing NULL terminator to be sure */
	szDataSizeBfr[4] = 'X';

   strncpy(stHead->acDataSize, szDataSizeBfr, 4);
   stHead->acDelimeter[0] =  '|';
}

void CloseSockets(int *sockServer, int *sockClient){
	/* Close the sockets, then assign a secure exit value */
	close(*sockServer); *sockServer = -1;
   close(*sockClient); *sockClient = -1;
}

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



