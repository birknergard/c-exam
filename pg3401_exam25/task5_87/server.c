/*
 * TITLE: Task 5 Server
 * AUTHOR: 87
 * DESCRIPTION
 *
 * TODO: Logging for each error
 * TODO: Make rough complete smtp transaction, then implement proper validation
 * */
#include <stdio.h>
#include <stdlib.h>

#include "ewpdef.h"
#include "server.h"

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
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
   int iStatus = 0;
   int iListened;
   int iExiting = 0;

   /* Client Info */
   int sockClient = -1;
   struct sockaddr_in saClientAddress = {0};
   /* Declaring boundaries */
   int iMaxCMD = 100;

   /* Verification variables */
   int iIllegalCharFound;
   int iValidFileExtension;
   
   /* Reply structs */
   EWA_PROTOCOL ewaServerACCEPT = {0};
   EWA_PROTOCOL ewaServerREPLY = {0};

   /* Request structs (from client) */
   struct EWA_EXAM25_TASK5_PROTOCOL_CLOSECOMMAND ewaClientCLOSE = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD ewaClientCMD = {0};

   /* Checking if arguments were provided */
   if(iArgC == 0){
      perror("Server attempted start without arguments.");
      return 1;
   }

   /* Parse program arguments 
      If flag was given, store id from arguments */
   if(strcmp(arrpszArgV[1], "-port") == 0){
      iPortNumber = atoi(arrpszArgV[2]);
   }

   if(strcmp(arrpszArgV[3], "-id") == 0){
      strncpy(szServerID, arrpszArgV[4], MAX_ID);
      szServerID[strlen(szServerID)] = '\0';
   }

	/* Open network socket (TCP/IP Protocol), as a stream */
	sockServer = socket(AF_INET, SOCK_STREAM, 0);
	if(sockServer < 0){
		printf("Error when opening socket - errcode: %d", errno);
		close(sockServer);
      sockServer = -1;
		return 1;
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
		close(sockServer);
      sockServer = -1;
		return 1;
	} 

	/* Make server listen for input. 
    * Since its only one client we set listen to 1 */
	iListened =	listen(sockServer, 1);
	if(iListened < 0){
		printf("Listen failed - errcode: %d", errno);
		close(sockServer); 
      sockServer = -1;
		return 1;
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
      /* Since both sockets are open we use this helper function instead */
      CloseSockets(&sockServer, &sockClient);
      return 1;
   }
   
   /* ACCEPT: Create server reply protocol */
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
      return 1;
	}  

   /* TODO: Add error messages exit conditions */
   RunProtocolHELO(&sockClient);

   RunProtocolMAILFROM(&sockClient);

   RunProtocolRCPTTO(&sockClient);

   /* ---CMD PROTOCOL START--- */
   /* From here on the program loops until the client sends a close request or an error occurs.
    * Set upper boundary to three while developing. */
   int k = 0;
   while(++k <= iMaxCMD){

      /* EXIT: PROGRAM EXIT */
      if(iStatus < 0 || iExiting == 1){
         break;
      }

      /* Sets limit on CMDs */
      if(k == iMaxCMD){
         printf("HIT MAX DATA LOOPS");
         break;
      }

      /* DATACMD/CLOSE: Retrieve as data cmd at first, 
       * but checking command for actual protocol */
      if(recv(sockClient, &ewaClientCMD, sizeof(ewaClientCMD), 0) < 0){
         printf("%s: Receive failed! errcode - %d\n", szServerID, errno);
         iStatus = -1;
         continue;
      } 
      printf("DATACMD REQ: %s\n", (char *) &ewaClientCMD);


      if(VerifyHeader(ewaClientCMD.stHead, 64, 0) <= 0){
         HandleServerError(&sockClient, &ewaServerREPLY, MSG_ERROR, "DENIED HEADER");
         iStatus = 1;
         continue;
      }

      /* Storing command in this buffer */
      char szCommand[5] = {0}; 
      snprintf(szCommand, 5, "%s", ewaClientCMD.acCommand);


      /* QUIT: If the command is quit, we exit the program completely */
      if(strcmp(szCommand, "QUIT") == 0){
         CreateServerReply(&ewaServerREPLY, MSG_EXIT, "QUIT RECEIVED: EXITING SERVER");
         if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
            iStatus = -1;
         }
         printf("EXITING SERVER\n");
         continue;
      }

      /* DATA: If command is data we run the data protocol */
      if(strcmp(szCommand, "DATA") == 0){
         iStatus = RunProtocolDATA(&sockClient, &ewaClientCMD);

         char szStatus[20] = {0};
         switch(iStatus) {
            case 0: strcpy(szStatus, "OK\0"); break;
            case 1: strcpy(szStatus, "PROTOCOL ERROR\0"); break;
            case -1: strcpy(szStatus, "SEND ERROR\0"); break;
            case -2: strcpy(szStatus, "RECV ERROR\0"); break;
            case -3: strcpy(szStatus, "MALLOC ERROR\0"); break;
            case -4: strcpy(szStatus, "FILE ERROR\0"); break;

            default: strcpy(szStatus, "UNKNOWN ERROR\0"); break;
         }
         printf("PROTOCOL DATA -> %s", szStatus);

      } else {
         iStatus = HandleServerError(&sockClient, &ewaServerREPLY, MSG_ERROR, "INVALID COMMAND");
         continue;
      }
   }/*-> COMMAND LOOP */

   /* Cleanup and exit */
   CloseSockets(&sockServer, &sockClient);
   return 1;
}

/*
 * Since im going to verify my own structs, but rather the client structs,
 * this function takes a void* of the header struct
 * */
int CreateServerReply(EWA_PROTOCOL *ewaStruct, char szStatusCode[], char szFormat[], ... ){
	va_list vaArgs;

   /* Reset struct */
   memset(ewaStruct, 0, sizeof(64));

   /* Add magic number */
   strncpy(ewaStruct->stHead.acMagicNumber, EWA_EXAM25_TASK5_PROTOCOL_MAGIC, 3);

   /* Need snprint for converting sizeof data to ascii with padding, but it adds a null terminator to the string when used.
    * Therefore we first create the string we want and then copy everything but the null terminator to the struct 
    * Also pad it to four bytes of ASCII by adding leading zeroes */

   char szDataSizeBfr[5];
   snprintf(szDataSizeBfr, 5, "%04d");
   /* Removing NULL terminator to be sure */
	szDataSizeBfr[4] = 'X';

   strncpy(ewaStruct->stHead.acDataSize, szDataSizeBfr, 4);
   ewaStruct->stHead.acDelimeter[0] =  '|';

   strncpy(ewaStruct->acStatusCode, szStatusCode, 3);

   ewaStruct->acHardSpace[0] = ' ';


	char szBuffer[SERVER_MSGSIZE];
   memset(szBuffer, 0, SERVER_MSGSIZE);

	va_start(vaArgs, szFormat);
   /* Adding extra space for zero terminator so it doesn't truncate any data */
	vsnprintf(szBuffer, SERVER_MSGSIZE + 1, szFormat, vaArgs);
	va_end(vaArgs);

   /* Removes zero terminator for good measure */
	szBuffer[strcspn(szBuffer, "\0")] = 0;

   /* Copies the given size argument x bytes of the created string into the struct. */
   strncpy(ewaStruct->acFormattedString, szBuffer, SERVER_MSGSIZE);
   ewaStruct->acHardZero[0] = '\0';

   ewaStruct = NULL;

   return 0;
}

/*
 * Performs a bunch of checks on the header structure to see if matches protocol
 * Returns the data size from header if accepted, otherwise returns -1.
 * */
int VerifyHeader(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER stHead, int iByteLimit, int iPrint){
   /* Declaring variables */
   int iSize = 0, i;
   char szBuf[4];
   memset(szBuf, 0, 4);

   /* Check structure */ 
   if(strncmp(stHead.acMagicNumber, "EWP", 3) != 0) return -1;
   if(iPrint == 1) printf("-> MAGIC OK");

   /* Checks if every byte is a digit. Only reads ever reads 4 digits. */
   for(i = 0; i < 4; i++){
      if(isdigit(stHead.acDataSize[i]) == 0){
         return -1;
      }
   }
   if(iPrint == 1) printf("DIGITS -> OK");

   /* Ensures only 4 digits are read */
   strncpy(szBuf, stHead.acDataSize, 4);

   /* Convert to int */
   iSize = atoi(szBuf);

   /* Check if value exceeds max (given as argument) */
   printf("Checking WITHIN BYTELIMIT;");
   if(iSize > iByteLimit) return -1;
   if(iPrint == 1) printf("DIGITS -> OK");

   if(stHead.acDelimeter[0] != '|') return -1;
   if(iPrint == 1) printf("DIGITS -> OK");

   /* All checks passed */
   printf("HEADER VERIFIED\n");
   return iSize;
}

/*
 * This block of code is repeated a lot. Shorthand function for code clarity.
 * Attempts to sends an error message to the client, then closes sockets and exits with error. 
 *
 * NOTE: This function exits the program! Make sure to clean up before invoke
 * */
int HandleServerError(int *sockClient, EWA_PROTOCOL *ewaStruct, char szCode[], char szResponse[]){
   
   /* Create the response on the protocol address */
   CreateServerReply(ewaStruct, szCode, szResponse);

   /* If message fails to send, the server shuts down */
   if(send(*sockClient, ewaStruct, sizeof(EWA_PROTOCOL), MSG_DONTWAIT) < 0){
      printf("\nSEND FAILED - errcode %d\n", errno);
      return -1;
   }

   return 1;
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

int RunProtocolHELO(int *sockClient){
   char szClientID[MAX_ID];
   char szClientIP[IP_STRING_SIZE]; /* IP as string */
   long int liClientIP; /* IP as raw number */
   int iIDSize = 0;

   EWA_PROTOCOL ewaServerHELO = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO ewaClientHELO = {0};

   /* HELO: Receive request */
	if(recv(*sockClient, &ewaClientHELO, sizeof(ewaClientHELO), 0) < 0){
		return -1;
	} 
   /* Print to terminal */
   printf("HELO REQ: %s\n", (char *) &ewaClientHELO);

   /* HELO: Validate client request */
   /* CHECKLIST FOR VALIDATION:
    *  1. VALID HEADER (used for every header after this):
    *    -> MAGIC = 'EWA'
    *    -> DELIMITER = '|'
    *    -> BYTE COUNT:
    *       -> IS 4 DIGIT NUMBER (WITH 0 PADDING)
    *       -> TODO:
    *
    *  2. REQUEST BODY:
    *    -> COMMAND = "HELO"*/
   if((iIDSize = VerifyHeader(ewaClientHELO.stHead, sizeof(ewaClientHELO), 0)) < 0) {/* NOT VALID header */
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID HEADER");
      return 1;
   }

   /* NOT HELO COMMAND */
   if(strncmp(ewaClientHELO.acCommand, "HELO", 4) != 0){ /* NOT HELO command */
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID COMMAND. \"HELO\" EXPECTED");
      return 1;
   } 
   printf("-> OK\n");

   /* NOT ASCI 0x20 HARDSPACE */
   if(ewaClientHELO.acHardSpace[0] != 0x20 ){ /* NOT ASCI value for space */
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID FORMAT");
      return 1;
   }
   printf("-> OK\n");

   /* STRING SIZE OUT OF BOUNDS */
   if(sizeof(ewaClientHELO.acFormattedString) > 51 || sizeof(ewaClientHELO.acFormattedString) < 0){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID REQUEST STRING SIZE");
      return 1;
   }
   printf("-> OK\n");

   if(ewaClientHELO.acHardZero[0] != '\0' /* Zero terminator */){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "MISSING ZERO TERMINATOR");
      return -1;
   }
   printf("-> OK\n");

   /* Initialize local variables for client data */
   liClientIP = 0;
   memset(szClientID, 0, MAX_ID);

   /* Attempt to parse username into local buffer */
   int iFormatCRead;
   for(iFormatCRead = 0; iFormatCRead < iIDSize - 8; iFormatCRead++){
      if(ewaClientHELO.acFormattedString[iFormatCRead] == '.'){
         szClientID[iFormatCRead] = '\0';
         break;
      }
      szClientID[iFormatCRead] = ewaClientHELO.acFormattedString[iFormatCRead]; 
   }

   /* ID needs to be at least two characters, 
    * max 42 (given that we need address as well, which is minimum 8 (1.1.1.1 */
   if(strlen(szClientID) < 2 || strlen(szClientID) > 40){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR,"INVALID CLIENT ID");
      return 1;
   }

   /* Check if remaining string is long enough to hold an IP address with . separators */
   if((strlen(ewaClientHELO.acFormattedString) - strlen(szClientID)) < 7){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR,"MISSING IP ADDRESS");
      return 1;
   }

   /* Parsing IP address. Starts calculating from where the id string ended (skipping the . terminator)
    * Keeping it both as a string and in raw format */
   liClientIP = ParseIPv4Address(ewaClientHELO.acFormattedString + (iFormatCRead + 1));
   GetIPv4AddressAsString(szClientIP, htonl(liClientIP));

   /* Reset for later use */
   iFormatCRead = 0;

   /* Verify IP address */
   if(liClientIP <= 0){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID IP ADDRESS");
      return 1;
   }

   /* HELO: If all of these conditions are met, set reply to OK */
   CreateServerReply(&ewaServerHELO, MSG_ACCEPT, "%d HELLO %s", liClientIP, szClientID);

   /* HELO: Send response to client */
	if(send(*sockClient, &ewaServerHELO, 64, 0) < 0){
		printf("%s: SEND FAILED - errcode %d", errno);
      return -1;
	}  
   
   return 0;
}

int RunProtocolMAILFROM(int *sockClient){

   struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM ewaClientMAILFROM = {0};
   EWA_PROTOCOL ewaServerREPLY = {0};

   /* RECEIVE "MAIL_FROM PROTOCOL" */
	if(recv(*sockClient, &ewaClientMAILFROM, sizeof(ewaClientMAILFROM), 0) < 0){
		return -1;
	} 
   printf("MAIL FROM REQ: %s\n", (char *) &ewaClientMAILFROM);
   /* TODO: Validation of MAIL FROM TO*/
   

   /* MAIL FROM: Sending response to client */
   CreateServerReply(&ewaServerREPLY, MSG_ACCEPT, "MAIL FROM RECEIVED");
	if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
      return -1;
	}  

   return 0;
}

int RunProtocolRCPTTO(int *sockClient){

   struct EWA_EXAM25_TASK5_PROTOCOL_RCPTTO ewaClientRCPTTO = {0};
   EWA_PROTOCOL ewaServerREPLY = {0};

   /* RCPT TO: Receive request */
	if(recv(*sockClient, &ewaClientRCPTTO, sizeof(ewaClientRCPTTO), 0) < 0){
		return -1;
	} 
   printf("RCPT TO REQ: %s\n", (char *) &ewaClientRCPTTO);
   /* TODO: Validation of RCPT TO*/


   /* RCPT TO: Send response */
   CreateServerReply(&ewaServerREPLY, MSG_ACCEPT, "RCPT TO RECEIVED - READY FOR COMMAND");
	if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
      return -1;
	}  

   return 0;
}

/* returns error code
 * 0 = OK
 * 1 = PROTOCOL ERROR
 * -1 = SEND ERROR
 * -2 = RECV ERROR
 * -3 = MALLOC ERROR
 * -4 = FILE ERROR
 * */
int RunProtocolDATA(int *sockClient, void *ewaCMD){
   int iMaxDATA = 500;
   int iStatus = 0;

   /* Datafile protocol variables   */
   int iFileNameBytesToRead;
   int iTotalBytesRemaining = 0;

   EWA_PROTOCOL ewaServerREPLY = {0};

   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD ewaClientDATACMD = 
      *(struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD *) ewaCMD;

   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER ewaClientFILEHEAD = {0}; 
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE *ewaClientFILE = NULL; /* Pointer because data is dynamic*/

   puts("VERIFYING FILENAME\n");
   /* Creating zero terminated buffer, to retrieve strlen
    * (cant retrieve strlen since acFormattedString isnt zero terminated) */
   char szFileNameBuffer[50];
   strncpy(szFileNameBuffer, ewaClientDATACMD.acFormattedString, 50);
   szFileNameBuffer[49] = '\0';
   int iFormattedStringLength = strlen(szFileNameBuffer);

   char szFileExtension[5] = {0};

   /* First, check for illegal symbols (Only reading as many bytes as was verified in head) */
   int iIllegalCharFound = 0;
   int j = 0, u;
   for(u = 0; u < iFormattedStringLength; u++){
      if(ewaClientDATACMD.acFormattedString[u] == '/'){
         iIllegalCharFound = 1;
         break;
      }

      /* Checks if there are null terminators anywhere but at acHardZero */
      if(ewaClientDATACMD.acFormattedString[u] == '\0' &&
         &ewaClientDATACMD.acFormattedString[u] != &ewaClientDATACMD.acHardZero[0]){
         iIllegalCharFound = 1;
         break;
      }

      /* Check for .eml file extension, when at the last 4 characters of string */
      if(u >= (iFormattedStringLength - 4)){
         szFileExtension[j] = ewaClientDATACMD.acFormattedString[u];
         j++;
      }
   }

   szFileExtension[4] = '\0';
   int iValidFileExtension = strncmp(szFileExtension, ".eml", 4);

   if(iIllegalCharFound != 0){
      if(HandleServerError(sockClient, &ewaServerREPLY, MSG_ERROR, "DATACMD - ILLEGAL CHARACTER") > 0)
         return -1;
      else return 1;
   }

   if(iValidFileExtension != 0){
      if(HandleServerError(sockClient, &ewaServerREPLY, MSG_ERROR, "DATACMD - INVALID FILETYPE") > 0);
      return 1;
   }

   /* Create filename at current directory (./<FILENAME>). Reusing buffer from validation step */
   char szFile[64] = "./emlfiles/"; 
   strcat(szFile, szFileNameBuffer);


   CreateServerReply(&ewaServerREPLY, MSG_OK, "FILENAME OK, READY FOR DATA");

   /* DATACMD: Send reply  */
   if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
      return -1;
   }  

   puts("CHECKING INITIAL HEADER\n");
   /* FILE: Verify the head first */
   if(recv(*sockClient, &ewaClientFILEHEAD, sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER), MSG_PEEK) < 0){
      return -2;
   } 

   /* DATAFILE: Verify first header before loop entry */
   iTotalBytesRemaining = VerifyHeader(ewaClientFILEHEAD, MAX_READ, 1);
   if(iTotalBytesRemaining < 0) {
      return HandleServerError(sockClient, &ewaServerREPLY, MSG_ERROR, "DATAFILE - REQUEST DENIED");
   }

   puts("ACCEPTING INITIAL DATA\n");
   CreateServerReply(&ewaServerREPLY, MSG_ACCEPT, "DATAFILE ACCEPT - READY");
   if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
      return -1;
   }

   /* If there's no problems with initial header, start receiving data */

   /* Declaring variables for loop */
   char *pszBuffer = NULL;
   int iBufferSize = 0;
   int iTotalBytesChecked = 0;
   int iEOF = 0;

   puts("STARTING DATAFILE PROTOCOL\n");

   /*----DATAFILE RECEIVE LOOP----*/
   while(1){ 

      /* Exits the datafile protocol, returning the errorcode */
      if(iStatus != 0) return iStatus;

      /* DATAFILE: Allocates struct based on header size (NOTE: Excluding header */
      ewaClientFILE = (struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE *) malloc(iTotalBytesRemaining);
      if(ewaClientFILE == NULL){
         iStatus = HandleServerError(sockClient, &ewaServerREPLY, MSG_ERROR, "DATAFILE INVALID SIZE");
         continue;
      }

      puts("HEADER VERIFIED - RECIEVING DATA");
      /* DATAFILE: Receives data struct based on header size */
      if(recv(*sockClient, &ewaClientFILE, iTotalBytesRemaining, MSG_WAITALL) < 0){
         iStatus = -1;
         continue;
      } 

      /* DATAFILE: Make sure data size matches header by checking for null terminator*//*
      if(ewaClientFILE->acFileContent[iTotalBytesRemaining - 8] != '\0'){
         puts("ERROR: BODY DOES NOT MATCH SIZE OF HEADER");
         iStatus = HandleServerError(sockClient, &ewaServerREPLY, MSG_ERROR, "BODY DOES NOT MATCH SIZE OF HEADER");
         continue;
      }
      */

      printf("FILE IS %d BYTES\n", iTotalBytesRemaining);
      /* ----DATAFILE READ ENTRY----*/
      while(iTotalBytesRemaining > 0){

         /* Exits datafile read if error occured */
         if(iStatus != 0) break;

         /* If FileContent size is greater than MAX_READ, we read MAX_READ at a time */
         if(iTotalBytesRemaining > MAX_READ){
            iBufferSize = MAX_READ;

         /* Until the bytes to read is smaller than max read, where we read the remainig bytes */
         } else {
            /* Make one byte bigger, so we can safely zero terminate the string */
            iBufferSize = iTotalBytesRemaining + 1;
         }

         printf("DATAFILE: Reading segment of size %d - %d Remaining ...\n", iBufferSize - 1, iTotalBytesRemaining);


         /* Allocate the buffer */
         pszBuffer = (char *) malloc(iBufferSize);
         if(pszBuffer == NULL){
            free(ewaClientFILE);
            ewaClientFILE = NULL;

            iStatus = -3;
            continue;
         }
         memset(pszBuffer, 0, iBufferSize);

         /* Write "iBufferSize" count of */
         char *carrWindow = (char *) malloc(4);
         if(carrWindow == NULL){
            return -3;
         }
         memset(carrWindow, 20, 4);
         puts("ALLOCATED BUFFER AND WINDOW");

         /* Checks entire buffer */
         int i;
         puts("STARTING READ");
         for(i = 0; i < iBufferSize; i++){

            puts("MOVING WINDOW");
            /* Create a three byte "window" at a time. That way we can check for the "\n.\n" exit pattern */
            printf("2 -> %d \n", iTotalBytesChecked + i);
            carrWindow[2] = ewaClientFILE->acFileContent[iTotalBytesChecked + i];
            puts("1");
            if(i >= 1) carrWindow[1] = ewaClientFILE->acFileContent[iTotalBytesChecked + i - 1];
            puts("0");
            if(i >= 2) carrWindow[0] = ewaClientFILE->acFileContent[iTotalBytesChecked + i - 2];

            /* End with a null terminator so we can compare as string */
            carrWindow[3] = '\0';
            printf("DATAFILE: %d WINDOW=%s\n", iTotalBytesChecked + i, carrWindow);

            /* Reads from right to left, starting at the third index.
             * Only checks after iteration 2 to prevent reading empty indexes (0, 1) */
            if(strncmp(carrWindow, "\n.\n", 3) == 0 && i >= 2){
               iEOF = 1;

               /* We break here so the buffer ends at . */
               break;
            }

            /* If EOF isn't hit, we write one byte to the buffer
             * We write the leftmost entry in the window */
            pszBuffer[i] = carrWindow[3];
         }

         free(carrWindow);
         carrWindow = NULL;
         /* Once we've checked the buffer, we write to the file
          * We reduce the read count for every time we write data to file */
         if(iBufferSize < MAX_READ){
            pszBuffer[iBufferSize] = '\0';
         }

         /* DATAFILE: Open the file in (a)ppend mode (new data is appended to the end of the file) */
         FILE *fpClientFile = fopen(szFile, "a");

         /* Write the data from the buffer to the file */  
         iTotalBytesRemaining -= fprintf(fpClientFile, pszBuffer);
         /*fprintf(fpClientFile, pszBuffer); */

         /* Close the file */
         fclose(fpClientFile);
         fpClientFile = NULL;

         /* Track how many bytes were read */
         iTotalBytesChecked += iBufferSize;

         /* Free the buffer */
         free(pszBuffer);
         pszBuffer = NULL;

         /* If we hit EOF during the parse we exit the loop here. But first we let the client know we are done */
         if(iEOF == 1){
            free(ewaClientFILE);
            ewaClientFILE = NULL;

            /* EXIT DATAFILE READ */
            break;
         } 
      }


      /* DATAFILE: When done with successful request, send response to continue */
      switch(iEOF){
         case 0: 
            CreateServerReply(&ewaServerREPLY, MSG_ACCEPT, "DATAFILE OK - READY");
            break;

         case 1: 
            CreateServerReply(&ewaServerREPLY, MSG_OK, "DATAFILE OK - WRITE COMPLETE");
            break;
      }

      /* DATAFILE: Send message with code defined above */
      if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
         iStatus = -1;
         continue;
      }

      /* DATAFILE: If iEOF was hit we exit this loop and return to start of DATACMD :) */
      if(iEOF == 1){
         continue; /* Returns to DATACMD/QUIT LOOP */
      }
         
      /* DATAFILE: Receive new header */
      if(recv(*sockClient, &ewaClientFILEHEAD, sizeof(EWA_HEAD), MSG_PEEK) < 0){
         iStatus = -1;
         continue;
      } 

      /* DATAFILE: Verify and reenter loop */
      iTotalBytesRemaining = VerifyHeader(ewaClientFILEHEAD, MAX_FILE, 1);
      if(iTotalBytesRemaining < 0){
         iStatus = HandleServerError(sockClient, &ewaServerREPLY, MSG_ERROR, "INVALID HEADER OR SIZE");
         continue;
      }

      CreateServerReply(&ewaServerREPLY, MSG_OK, "FILEDATA ACCEPTED - READY");
      if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
         iStatus = -1;
         continue;
      }/*-> NEW HEADER VERIFIED. RESTARTING DATAFILE LOOP */
   }/*-> DATA LOOP */

   return 0;
}
