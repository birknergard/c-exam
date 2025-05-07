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
	int sockServer = -1;
	struct sockaddr_in saServerAddress = {0};
	int iNewAddressLength;

   char szServerID[MAX_ID] = {0};  
   int iPortNumber = -1;
   long int iServerAddress = 0x7F000001;

   /* Server status */
   int iListened;

   /* Client Info */
   int sockClient = -1;
	struct sockaddr_in saClientAddress = {0};
   char szClientID[MAX_ID];
   char szClientIP[IP_STRING_SIZE]; /* IP as string */
   long int liClientIP; /* IP as raw number */

   /* Declaring iterators */
   int i, j;

   /* Verification variables */
   int iBytesToRead;
   int iIllegalCharFound;
   int iValidFileExtension;
   
   /* Other */


   /* Reply structs */
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT ewaServerACCEPT = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERHELO ewaServerHELO = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY ewaServerREPLY = {0};

   /* Request structs (from client) */
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO ewaClientHELO = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM ewaClientMAILFROM = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_RCPTTO ewaClientRCPTTO = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD ewaClientDATACMD = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER ewaClientFILEHEAD = {0}; 
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE *ewaClientFILE = NULL; /* Pointer because data is dynamic*/
   struct EWA_EXAM25_TASK5_PROTOCOL_CLOSECOMMAND ewaClientCLOSE = {0};

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
      strncpy(szServerID, arrpszArgV[4], MAX_ID);
      szServerID[strlen(szServerID)] = '\0';
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
	if(bind(sockServer, (struct sockaddr *) &saServerAddress, sizeof(saServerAddress)) < 0){
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
   
   CreateServerReply(&ewaServerACCEPT, "220", "%X %s %s - %s:%s:%s, %ld",
      iServerAddress,
      "SMTP", 
      szServerID, 
      "06",
      "05",
      "2025",
      (int)time(NULL)
   );

	/* ACCEPT: Send response */
	if(send(sockClient, &ewaServerACCEPT, sizeof(ewaServerACCEPT), 0) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
	}  

   /* HELO: Receive request */
	if(recv(sockClient, &ewaClientHELO, sizeof(ewaClientHELO), 0) < 0){
		printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
		return -1;
	} 
   printf("HELO REQ: %s\n", (char *) &ewaClientHELO);

   /* HELO: Validate client request */
   /* CHECKLIST:
    *  1. VALID HEADER:
    *    -> MAGIC = 'EWA'
    *    -> DELIMITER = '|'
    *    -> BYTE COUNT:
    *       -> IS 4 DIGIT NUMBER (WITH 0 PADDING)
    *       -> TODO:
    *
    *  2. REQUEST BODY:
    *    -> COMMAND = "HELO"*/
   if((iBytesToRead = VerifyHeader(ewaClientHELO.stHead, sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO))) > 0) {
      if( /* Verify body structure */
         (strncmp(ewaClientHELO.acCommand, "HELO", 4) == 0) &&
         ewaClientHELO.acHardSpace[0] == 0x20 && /* ASCI value for space */
         sizeof(ewaClientHELO.acFormattedString) <= 50 && sizeof(ewaClientHELO.acFormattedString) > 0 &&
         ewaClientHELO.acHardZero[0] == '\0' /* Zero terminator */
      ){
         /* Initialize local variables for client data */
         liClientIP = 0;
         memset(szClientID, 0, MAX_ID);

         /* Attempt to parse username into local buffer */
         for(i = 0; i < iBytesToRead - 8; i++){
            if(ewaClientHELO.acFormattedString[i] == '.'){
               szClientID[i] = '\0';
               break;
            }
            szClientID[i] = ewaClientHELO.acFormattedString[i]; 
         }

         /* ID needs to be at least two characters, 
          * max 42 (given that we need address as well, which is minimum 8 (1.1.1.1 */
         if(strlen(szClientID) >= 2 && strlen(szClientID) < 40){

            /* Check if remaining string is long enough to hold an IP address with . separators */
            if((strlen(ewaClientHELO.acFormattedString) - strlen(szClientID)) > 7){

               /* Parsing IP address. Starts calculating from where the id string ended (skipping the . terminator)
                * Keeping it both as a string and in raw format */
               liClientIP = ParseIPv4Address(ewaClientHELO.acFormattedString + (i + 1));
               GetIPv4AddressAsString(szClientIP, htonl(liClientIP));

               /* Verify IP address */
               if(liClientIP > 0){
                  /* If all of these conditions are met, set reply to OK */
                  CreateServerReply(
                     &ewaServerHELO, EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_OK,
                     "%d HELLO %s", liClientIP, szClientID
                  );
               } else CreateServerReply(&ewaServerHELO,"501", "INVALID IP ADDRESS");
            } else CreateServerReply(&ewaServerHELO,"501", "NO IP ADDRESS PROVIDED");
         } else CreateServerReply(&ewaServerHELO,"501", "CLIENT ID IS INVALID");
      } else CreateServerReply(&ewaServerHELO,"501", "BAD REQUEST");
   } else CreateServerReply(&ewaServerHELO,"501", "NO IP ADDRESS PROVIDED");


   /* HELO: Send response to client */
	if(send(sockClient, &ewaServerHELO, 64, 0) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
      return -1;
	}  

   /* RECEIVE "MAIL_FROM PROTOCOL" */
	if(recv(sockClient, &ewaClientMAILFROM, sizeof(ewaClientMAILFROM), 0) < 0){
		printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
		return -1;
	} 
   printf("MAIL FROM REQ: %s\n", (char *) &ewaClientMAILFROM);
   /* TODO: Validation of MAIL FROM TO*/
   

   /* MAIL FROM: Sending response to client */
   CreateServerReply(&ewaServerREPLY, "250", "MAIL FROM RECEIVED");
	if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
      return -1;
	}  


   /* RCPT TO: Receive request */
	if(recv(sockClient, &ewaClientRCPTTO, sizeof(ewaClientRCPTTO), 0) < 0){
		printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
		return -1;
	} 
   printf("RCPT TO REQ: %s\n", (char *) &ewaClientRCPTTO);
   /* TODO: Validation of RCPT TO*/

   /* RCPT TO: Send response */
   CreateServerReply(&ewaServerREPLY, "250", "RCPT TO RECEIVED");
	if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
		printf("%s: SEND FAILED - errcode %d", szServerID, errno);
      CloseSockets(&sockServer, &sockClient);
      return -1;
	}  

   /* ---DATA PROTOCOL START--- */
   /* From here on the program loops until the client sends a close request*/
   for(;;){
      char ewaANON[12] = {0};

      /* DATACMD/CLOSE: Retrieve as anonymous byte array to retrieve command */
      if(recv(sockClient, &ewaANON, sizeof(64), 0) < 0){
         printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
         CloseSockets(&sockServer, &sockClient);
         return -1;
      } 

      /* Verify header */
      if(VerifyHeader(()) > -1) 

      if(recv(sockClient, &ewaClientDATACMD, sizeof(ewaClientDATACMD), 0) < 0){
         printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
         CloseSockets(&sockServer, &sockClient);
         return -1;
      } 
      printf("DATACMD REQ: %s\n", (char *) &ewaClientDATACMD);

      /* DATA: Verifying filename is valid.
       *    MUST HAVE: .eml
       *    MUST NOT HAVE: '/' or '\0' */
      /* Initializing control variables */
      iBytesToRead = 0;
      iIllegalCharFound = 0;
      iValidFileExtension = 0;
      char szFileNameBuffer[50] = {0};

      if((iBytesToRead = VerifyHeader(ewaClientDATACMD.stHead, 64)) != -1){
         printf("HEADER->OK;");

         /* Creating zero terminated buffer, to retrieve strlen
          * (cant retrieve strlen since acFormattedString isnt zero terminated) */
         strncpy(szFileNameBuffer, ewaClientDATACMD.acFormattedString, 50);
         szFileNameBuffer[49] = '\0';
         int iFormattedStringLength = strlen(szFileNameBuffer);

         char szFileExtension[5] = {0};
         j = 0;

         /* First, check for illegal symbols (Only reading as many bytes as was verified in head) */
         for(i = 0; i < iFormattedStringLength; i++){
            if(ewaClientDATACMD.acFormattedString[i] == '/'){
               iIllegalCharFound = -1;
               break;
            }
            /* Checks if there are null terminators anywhere but at acHardZero */
            if(ewaClientDATACMD.acFormattedString[i] == '\0' &&
               &ewaClientDATACMD.acFormattedString[i] != &ewaClientDATACMD.acHardZero[0]){
               iIllegalCharFound = -1;
               break;
            }

            /* Check for .eml file extension, when at the last 4 characters of string */
            if(i >= (iFormattedStringLength - 4)){
               szFileExtension[j] = ewaClientDATACMD.acFormattedString[i];
               j++;
            }
         }
         szFileExtension[4] = '\0';
         iValidFileExtension = strncmp(szFileExtension, ".eml", 4);

         if(iIllegalCharFound != -1){
            if(iValidFileExtension == 0){
               CreateServerReply(
                  &ewaServerREPLY,
                  EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY_READY,
                  "FILENAME OK, READY FOR HEADER"
               );
            } else CreateServerReply(&ewaServerREPLY, "501", "INVALID FILE TYPE");
         } else CreateServerReply(&ewaServerREPLY, "501", "CONTAINS ILLEGAL SYMBOLS");
      } else CreateServerReply(&ewaServerREPLY, "501", "DENIED HEADER/BYTE SIZE"); 

      /* Create filename at current directory (./FILENAME). Reusing buffer from validation step */
      char szClientFileName[64] = {'.', '/'}; 
      strcat(szClientFileName, szFileNameBuffer);

      /* DATACMD: Send reply  */
      if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
         printf("%s: SEND FAILED - errcode %d", szServerID, errno);
         CloseSockets(&sockServer, &sockClient);
      }  

      /* FILE: Receive the head first */
      if(recv(sockClient, &ewaClientFILEHEAD, sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER), 0) < 0){
         printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
         CloseSockets(&sockServer, &sockClient);
         return -1;
      } 
      printf("DATAFILE HEAD: %s\n", (char *) &ewaClientFILEHEAD);


      /* DATAFILE: Verify first header before loop entry */
      iBytesToRead = VerifyHeader(ewaClientFILEHEAD, MAX_READ);
      if(iBytesToRead == -1) {
         CreateServerReply(&ewaServerREPLY, "501", "FILE REQUEST DENIED - COULD NOT READ HEADER");
         if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
            printf("%s: SEND FAILED - errcode %d", szServerID, errno);
            CloseSockets(&sockServer, &sockClient);
            return -1;
         }

         /* If we fail to read the header we exit the loop */
         break;

      /* If there's no problems with initial header, start receiving data */
      } else {

         /* DATAFILE: Open file */
         FILE *fpClientFile = fopen(szClientFileName, "w+");
         int iBufferSize;
         int iEOF = 0;
         int iBytesChecked = 0;

         while(1){
            /* DATAFILE: Allocates struct based on header size */
            ewaClientFILE = (struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE *) malloc(iBytesToRead);
            if(ewaClientFILE == NULL){
               printf("%s: Malloc failed! errcode - %d\n", szServerID, errno);
               fclose(fpClientFile);
               CloseSockets(&sockServer, &sockClient);
               return -1;
            }

            /* DATAFILE: Receives data struct based on header size */
            if(recv(sockClient, ewaClientFILE, iBytesToRead, 0) < 0){
               printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
               free(ewaClientFILE);
               fclose(fpClientFile);
               fpClientFile = NULL;
               CloseSockets(&sockServer, &sockClient);
               return -1;
            } 

            /* DATAFILE:: Verify and write */
            while(iBytesToRead > 0){

               /* If FileContent size is greater than MAX_READ, we read MAX_READ at a time */
               if(iBytesToRead > MAX_READ){
                  iBufferSize = MAX_READ;
               } else {
                  /* Make one byte bigger, so we can safely zero terminate the string */
                  iBufferSize = iBytesToRead + 1;
               }

               char *pszBuffer = NULL;
               /* Allocate the buffer */
               pszBuffer = (char *) malloc(iBufferSize);
               if(pszBuffer == NULL){
                  printf("%s: Malloc failed! errcode - %d\n", szServerID, errno);
                  fclose(fpClientFile);
                  CloseSockets(&sockServer, &sockClient);
                  return -1;
               }

               /* Write "iBufferSize" count of */
               char carrWindow[3];
               memset(carrWindow, 0, 3);

               /* Checks entire buffer */
               for(i = 0; i < iBufferSize; i++){

                  /* Create a three byte "window" at a time. That way we can check for the "\n.\n" exit pattern */
                  carrWindow[i] = ewaClientFILE->acFileContent[iBytesChecked];
                  if(i >= 1) carrWindow[i-1] = ewaClientFILE->acFileContent[iBytesChecked-1];
                  if(i >= 2) carrWindow[i-2] = ewaClientFILE->acFileContent[iBytesChecked-2];


                  /* Reads from right to left, starting at the third index */
                  if(carrWindow == "\n.\n"){
                     iEOF = 1;

                     /* We break here so the buffer ends at . */
                     break;
                  }

                  /* If EOF isn't hit, we write one byte to the buffer */
                  pszBuffer[i] = ewaClientFILE->acFileContent[i];
                  iBytesChecked++;
               }

               /* Once we've checked the buffer, we write to the file
                * We reduce the read count for every time we write data to file */
               iBytesToRead -= fprintf(fpClientFile, pszBuffer, "%c");
               memset(pszBuffer, 0, iBufferSize);

               /* If we hit EOF during the parse we exit the loop here. But first we let the client know we are done */
               if(iEOF == 1){
                  free(pszBuffer);
                  pszBuffer = NULL;
                  break;
               } 
            }


            /* DATAFILE: When parsed request, free protocol struct and set to null */
            free(ewaClientFILE);

            ewaClientFILE = NULL;

            /* DATAFILE: When done with successful request, send response to continue */
            iBytesToRead = 0;
            switch(iEOF){
               case 0: 
                  CreateServerReply(&ewaServerREPLY, "354", "DATAFILE OK - READY");
                  break;

               case 1: 
                  CreateServerReply(&ewaServerREPLY, "354", "DATAFILE OK: WRITE COMPLETE");
                  break;
            }

            if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
               printf("%s: SEND FAILED - errcode %d", szServerID, errno);
               fclose(fpClientFile);
               fpClientFile = NULL;
               CloseSockets(&sockServer, &sockClient);
               return -1;
            }

            /* DATAFILE: If iEOF was hit we exit this loop and return to start of DATACMD :) */
            if(iEOF == 1){
               fclose(fpClientFile);
               fpClientFile = NULL;
               break;
            }
            
            /* DATAFILE: Receive new header */
            if(recv(sockClient, &ewaClientFILEHEAD, sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER), 0) < 0){
               printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
               fclose(fpClientFile);
               fpClientFile = NULL;
               CloseSockets(&sockServer, &sockClient);
               return -1;
            } 

            /* DATAFILE: Verify and reenter loop */
            iBytesToRead = VerifyHeader(ewaClientFILEHEAD, MAX_READ);
            if(iBytesToRead == -1){
               CreateServerReply(&ewaServerREPLY, "501", "FILE REQUEST DENIED - COULD NOT READ HEADER");
               if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
                  printf("%s: SEND FAILED - errcode %d", szServerID, errno);
                  fclose(fpClientFile);
                  fpClientFile = NULL;
                  CloseSockets(&sockServer, &sockClient);
                  return - 1;
               }
               break;
            }/*-> NEW HEADER VERIFIED */
         }/*-> DATAFILE LOOP */
      }/*-> INITIAL HEADER ACCEPTED */
   }/*-> DATACMD PROTOCOL STARTED (LOOP) */

   ewaClientFILE = NULL;
   CloseSockets(&sockServer, &sockClient);
   exit(1);
   return 1;
}

int CreateServerReply(void *vpStruct, char szStatusCode[], char szFormat[], ... ){
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY *ewaReply = 
      (struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY *) vpStruct;   
	va_list vaArgs;

   /* Reset struct */
   memset(ewaReply, 0, sizeof(64));

   /* Add magic number */
   strncpy(ewaReply->stHead.acMagicNumber, EWA_EXAM25_TASK5_PROTOCOL_MAGIC, 3);

   /* Need snprint for converting sizeof data to ascii with padding, but it adds a null terminator to the string when used.
    * Therefore we first create the string we want and then copy everything but the null terminator to the struct 
    * Also pad it to four bytes of ASCII by adding leading zeroes */

   char szDataSizeBfr[5];
   snprintf(szDataSizeBfr, 5, "%04d", sizeof(ewaReply));
   /* Removing NULL terminator to be sure */
	szDataSizeBfr[4] = 'X';

   strncpy(ewaReply->stHead.acDataSize, szDataSizeBfr, 4);
   ewaReply->stHead.acDelimeter[0] =  '|';

   strncpy(ewaReply->acStatusCode, szStatusCode, 3);

   ewaReply->acHardSpace[0] = ' ';


	char szBuffer[SERVER_MSGSIZE];
   memset(szBuffer, 0, SERVER_MSGSIZE);

	va_start(vaArgs, szFormat);
   /* Adding extra space for zero terminator so it doesn't truncate any data */
	vsnprintf(szBuffer, SERVER_MSGSIZE + 1, szFormat, vaArgs);
	va_end(vaArgs);

   /* Removes zero terminator for good measure */
	szBuffer[strcspn(szBuffer, "\0")] = 0;

   /* Copies the given size argument x bytes of the created string into the struct. */
   strncpy(ewaReply->acFormattedString, szBuffer, SERVER_MSGSIZE);
   ewaReply->acHardZero[0] = '\0';

   ewaReply = NULL;

   return 0;
}

/*
 * Performs a bunch of checks on the header structure to see if matches protocol
 * Returns the data size from header if accepted, otherwise returns -1.
 * */
int VerifyHeader(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER stHead, int iByteLimit){
   /* Declaring variables */
   int iSize = 0, i;
   char szBuf[4] = {0};

   /* Check structure */ 
   printf("Checking MAGIC");
   if(strncmp(stHead.acMagicNumber, "EWP", 3) != 0) return -1;
   printf("-> OK\n");

   /* Checks if every byte is a digit. Only reads ever reads 4 digits. */
   printf("Checking IF DIGITS");
   for(i = 0; i < 4; i++){
      if(isdigit(stHead.acDataSize[i]) == 0){
         return -1;
      }
   }
   printf("-> OK\n");
   /* Ensures only 4 digits are read */
   strncpy(szBuf, stHead.acDataSize, 4);

   /* Convert to int */
   iSize = atoi(szBuf);

   /* Check if value exceeds max (given as argument) */
   printf("Checking WITHIN BYTELIMIT;");
   if(iSize > iByteLimit) return -1;
   printf("-> OK\n");

   printf("Checking DELIMETER;");
   if(stHead.acDelimeter[0] != '|') return -1;
   printf("-> OK\n");

   /* All checks passed */
   printf("HEADER VERIFIED\n");
   return iSize;
}

void CloseSockets(int *sockServer, int *sockClient){
	/* Close the sockets, then assign a secure exit value */
	close(*sockServer); *sockServer = -1;
   close(*sockClient); *sockClient = -1;
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
            /*printf("String segment for bit field %d=%s - i=%d, j=%d\n", iCurrentBitField, szBuffer, i, j);*/
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

void GetIPv4AddressAsString(char *szDestination, long int liIPv4Address){
   char szBuffer[5];
   char szConverted[IP_STRING_SIZE];
   signed char byShiftedAddr;
   int i, j;

   memset(szConverted, 0, IP_STRING_SIZE);
   memset(szDestination, 0, IP_STRING_SIZE);

   /* Shitfing right, increasing by 8 for each byte up to 24, leaving 32 bits.*/
   j = 0;
   for(i = 0; i <= 24; i += 8){
      byShiftedAddr = liIPv4Address >> i;

      /* Resetting buffer */
      memset(szBuffer, 0, 5);

      j++;

      /* Creating string */
      if(j != 4)
         snprintf(szBuffer, 5, "%d.", byShiftedAddr);
      else 
         snprintf(szBuffer, 5, "%d", byShiftedAddr);

      /*printf("%d -> IPSTRING=%s\n", j, szBuffer);*/

      /* Concatenating buffer to converted string */
      strncat(szConverted, szBuffer, 4);  
   }

   strncpy(szDestination, (char*) szConverted, IP_STRING_SIZE);
   szDestination[strlen(szDestination)] = '\0';
}


