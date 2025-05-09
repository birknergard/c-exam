/*
 * TITLE: Task 5 Server
 * AUTHOR: 87
 * DESCRIPTION
 *
 *
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

   /* Reply structs */
   EWA_PROTOCOL ewaServerACCEPT = {0};
   EWA_PROTOCOL ewaServerREPLY = {0};

   /* Request structs (from client) */
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

   char szServerID[MAX_ID] = {0};  
   if(strcmp(arrpszArgV[3], "-id") == 0){
      memcpy(szServerID, arrpszArgV[4], MAX_ID - 1);
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

   printf("STARTED SERVER ON -PORT %d -ID %s\n", iPortNumber, szServerID);

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

   int iHELO = 0;
   char szClientID[50];
   memset(szClientID, 0, 50);
   long int liClientIP = -1;
   iHELO = RunProtocolHELO(&sockClient, szClientID, &liClientIP);
   if(iHELO != 0) {
      HandleServerError(&sockClient, &ewaServerREPLY, MSG_ERROR, "DENIED");
      CloseSockets(&sockServer, &sockClient);
      return 1;
   }

   puts("");

   int iMAILFROM = 0;
   char szMailFrom[50];
   memset(szMailFrom, 0, 50);
   iMAILFROM = RunProtocolMAILFROM(&sockClient, szMailFrom);
   if(iMAILFROM != 0) {
      HandleServerError(&sockClient, &ewaServerREPLY, MSG_ERROR, "DENIED");
      CloseSockets(&sockServer, &sockClient);
      return 1;
   }
   puts("");

   int iRCPTTO = 0;
   char szMailTo[50];
   memset(szMailFrom, 0, 50);
   iRCPTTO = RunProtocolRCPTTO(&sockClient, szMailTo);
   if(iRCPTTO != 0) {
      HandleServerError(&sockClient, &ewaServerREPLY, MSG_ERROR, "DENIED");
      CloseSockets(&sockServer, &sockClient);
      return 1;
   }

   puts("");

   /* ---CMD PROTOCOL START--- */
   /* From here on the program loops until the client sends a close request or an error occurs. */
   int n = 0;
   while(1){

      /* Hold for a moment */
      sleep(1);

      /* EXIT: PROGRAM EXIT */
      if(iExiting == 1){
         break;
      }

      if(iStatus > 0){
         /* Receiving to prevent hanging */
         if(recv(sockClient, &ewaClientCMD, sizeof(EWA_PROTOCOL), 0) < 0){
            iStatus = -2;
         } 
      }

      /* DATACMD/CLOSE: Retrieve as data cmd at first, 
       * but checking command for actual protocol */
      puts("ACCEPTING NEW COMMAND ...\n");
      if(recv(sockClient, &ewaClientCMD, sizeof(EWA_PROTOCOL), MSG_PEEK) < 0){
         printf("Receive error");
         iStatus = -2;
         continue;
      } 

      /* Storing command in this buffer */
      char szCommand[5] = {0}; 
      strncpy(szCommand, (char*) &(ewaClientCMD.acCommand), 4);
      szCommand[4] = '\0';

      /* QUIT: If the command is quit, we exit the program completely */
      if(strcmp(szCommand, "QUIT") == 0){
         puts("RECEIVED CMD -> QUIT");
         CreateServerReply(&ewaServerREPLY, MSG_EXIT, "QUIT RECEIVED: EXITING SERVER");
         if(send(sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
            iStatus = -1;
         }
         iExiting = 1;
         continue;
      }

      /* DATA: If command is data we run the data protocol */
      if(strcmp(szCommand, "DATA") == 0){
         puts("RECEIVED CMD -> DATA");
         iStatus = RunProtocolDATA(&sockClient);

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
         printf("\nPROTOCOL DATA -> %s\n\n", szStatus);


         if(iStatus < 0){
            HandleServerError(&sockClient, &ewaServerREPLY, MSG_ERROR, szStatus);
         }
         continue;
      }

      /* If we reach the command is invalid */
      printf("INVALID COMMAND %s\n", szCommand);
      HandleServerError(&sockClient, &ewaServerREPLY, MSG_EXIT, "INVALID COMMAND");
      iStatus = 1;
      iExiting = 1;
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
   memset(ewaStruct, 0, sizeof(EWA_HEAD));

   /* Add magic number */
   memcpy(ewaStruct->stHead.acMagicNumber, EWA_EXAM25_TASK5_PROTOCOL_MAGIC, 3);

   /* Need snprint for converting sizeof data to ascii with padding, but it adds a null terminator to the string when used.
    * Therefore we first create the string we want and then copy everything but the null terminator to the struct 
    * Also pad it to four bytes of ASCII by adding leading zeroes */

   ewaStruct->stHead.acDelimeter[0] =  '|';

   memcpy(ewaStruct->acStatusCode, szStatusCode, 3);

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
   memcpy(ewaStruct->acFormattedString, szBuffer, SERVER_MSGSIZE);
   ewaStruct->acHardZero[0] = '\0';

   char szDataSizeBfr[5];
   snprintf(szDataSizeBfr, 5, "%04ld", sizeof(*ewaStruct));

   memcpy(ewaStruct->stHead.acDataSize, szDataSizeBfr, 4);

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

   /* Check structure */ 
   if(strncmp(stHead.acMagicNumber, "EWP", 3) != 0) return 1;
   if(iPrint == 1) puts("  -> MAGIC OK");

   /* Checks if every byte is a digit. Only reads ever reads 4 digits. */
   for(i = 0; i < 4; i++){
      if(isdigit(stHead.acDataSize[i]) == 0){
         return 1;
      }
   }
   if(iPrint == 1) puts("  -> DIGITS OK");

   /* Ensures only 4 digits are read */
   char szBuf[5];
   memset(szBuf, 0, 5);
   strncpy(szBuf, stHead.acDataSize, 4);
   szBuf[4] = '\0';

   /* Convert to int */
   iSize = atoi(szBuf);

   /* Check if value exceeds max (given as argument) */
   if(iSize > iByteLimit) {
      return 1;
   }
   if(iPrint == 1) puts("  -> SIZE OK");

   if(stHead.acDelimeter[0] != '|') return 1;
   if(iPrint == 1) puts("  -> DELIMITER OK");

   /* All checks passed */
   puts("HEADER ACCEPTED");
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
         sprintf(szBuffer, "%d.", byShiftedAddr);
      else 
         snprintf(szBuffer, 4, "%d", byShiftedAddr);

      /*printf("%d -> IPSTRING=%s\n", j, szBuffer);*/

      /* Concatenating buffer to converted string */
      strncat(szConverted, szBuffer, 4);  
   }

   strncpy(szDestination, (char*) szConverted, IP_STRING_SIZE);
   szDestination[strlen(szDestination)] = '\0';
}

int RunProtocolHELO(int *sockClient, char szClientID[], long int *liClientIP){
   char szID[MAX_ID];
   char szIP[IP_STRING_SIZE]; /* IP as string */
   long int liIP; /* IP as raw number */
   int iIDSize = 0;

   EWA_PROTOCOL ewaServerHELO = {0};
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO ewaClientHELO = {0};

   /* HELO: Receive request */
	if(recv(*sockClient, &ewaClientHELO, sizeof(ewaClientHELO), 0) < 0){
		return -1;
	} 

   /* HELO: Validate client request */
   if((iIDSize = VerifyHeader(ewaClientHELO.stHead, sizeof(ewaClientHELO), 0)) < 0) {/* NOT VALID header */
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID HEADER");
      return 1;
   }

   /* NOT HELO COMMAND */
   if(strncmp(ewaClientHELO.acCommand, "HELO", 4) != 0){ /* NOT HELO command */
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID COMMAND. \"HELO\" EXPECTED");
      return 1;
   } 

   /* NOT ASCI 0x20 HARDSPACE */
   if(ewaClientHELO.acHardSpace[0] != 0x20 ){ /* NOT ASCI value for space */
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID FORMAT");
      return 1;
   }

   /* STRING SIZE OUT OF BOUNDS */
   if(sizeof(ewaClientHELO.acFormattedString) > 51 || sizeof(ewaClientHELO.acFormattedString) < 0){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID REQUEST STRING SIZE");
      return 1;
   }

   if(ewaClientHELO.acHardZero[0] != '\0' /* Zero terminator */){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "MISSING ZERO TERMINATOR");
      return 1;
   }

   /* Initialize local variables for client data */
   liIP = 0;
   memset(szID, 0, MAX_ID);

   /* Attempt to parse username into local buffer */
   int i;
   for(i = 0; i < iIDSize - 8; i++){
      if(ewaClientHELO.acFormattedString[i] == '.'){
         szID[i] = '\0';
         break;
      }
      szID[i] = ewaClientHELO.acFormattedString[i]; 
   }

   /* ID needs to be at least two characters, 
    * max 42 (given that we need address as well, which is minimum 8 (1.1.1.1 */
   if(strlen(szID) < 2 || strlen(szID) > 40){
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
   liIP = ParseIPv4Address(ewaClientHELO.acFormattedString + (i + 1));
   GetIPv4AddressAsString(szIP, htonl(liIP));

   /* Verify IP address */
   if(liIP <= 0){
	   HandleServerError(sockClient, &ewaServerHELO, MSG_ERROR, "INVALID IP ADDRESS");
      return 1;
   }

   /* HELO: If all of these conditions are met, set reply to OK */
   CreateServerReply(&ewaServerHELO, MSG_ACCEPT, "%d HELLO %s", liIP, szID);

   snprintf(szClientID, iIDSize, szID);
   *liClientIP = liIP;

   /* Print to terminal */
   printf("HELO: %s %d\n", szClientID, *liClientIP);
   /* HELO: Send response to client */
	if(send(*sockClient, &ewaServerHELO, 64, 0) < 0){
      return -1;
	}  
   
   return 0;
}

int RunProtocolMAILFROM(int *sockClient, char szMailFrom[]){

   struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM ewaClientMAILFROM = {0};
   EWA_PROTOCOL ewaServerREPLY = {0};

   /* RECEIVE "MAIL_FROM PROTOCOL" */
	if(recv(*sockClient, &ewaClientMAILFROM, sizeof(ewaClientMAILFROM), 0) < 0){
		return -2;
	} 
   int iSize;
   if((iSize = VerifyHeader(ewaClientMAILFROM.stHead, 64, 0)) < 0) return 1;

   if(ewaClientMAILFROM.acHardSpace[0] != ' '){
      return 1;
   }

   if(ewaClientMAILFROM.acHardZero[0] != '\0'){
      return 1;
   }

   /* Attempt to parse username into local buffer */
   int i, iFoundDomain;
   for(i = 0; i < iSize - 1; i++){
      if(ewaClientMAILFROM.acFormattedString[i] == '@' && i != iSize - 1){
         iFoundDomain = 1;
      }
      if(ewaClientMAILFROM.acFormattedString[iSize] == '>'){ 
        szMailFrom[i + 1] = '\0';
        break;
      }
      szMailFrom[i] = ewaClientMAILFROM.acFormattedString[i]; 
   }
   if(iFoundDomain != 1) return 1;

   if(strlen(szMailFrom) < 3) return 1; /* A valid email needs at least a name@domain */

   printf("MAILFROM: %s\n", szMailFrom);

   /* MAIL FROM: Sending response to client */
   CreateServerReply(&ewaServerREPLY, MSG_ACCEPT, "MAIL FROM RECEIVED");
	if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
      return -1;
	}  

   return 0;
}

int RunProtocolRCPTTO(int *sockClient, char szMailTo[]){

   struct EWA_EXAM25_TASK5_PROTOCOL_RCPTTO ewaClientRCPTTO = {0};
   EWA_PROTOCOL ewaServerREPLY = {0};

   /* RCPT TO: Receive request */
	if(recv(*sockClient, &ewaClientRCPTTO, sizeof(ewaClientRCPTTO), 0) < 0){
		return -1;
	} 

   int iSize;
   if((iSize = VerifyHeader(ewaClientRCPTTO.stHead, 64, 0)) < 0) return 1;

   if(ewaClientRCPTTO.acHardSpace[0] != ' '){
      return 1;
   }

   if(ewaClientRCPTTO.acHardZero[0] != '\0'){
      return 1;
   }

   /* Attempt to parse username into local buffer */
   int i, iFoundDomain;
   for(i = 0; i < iSize - 1; i++){
      if(ewaClientRCPTTO.acFormattedString[i] == '@' && i != iSize - 1){
         iFoundDomain = 1;
      }
      if(ewaClientRCPTTO.acFormattedString[iSize] == '>'){ 
        szMailTo[i + 1] = '\0';
        break;
      }
      szMailTo[i] = ewaClientRCPTTO.acFormattedString[i]; 
   }
   if(iFoundDomain != 1) return 1;

   if(strlen(szMailTo) < 3) return 1; /* A valid email needs at least a name@domain */

   printf("RCPTTO: %s\n", szMailTo);

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
int RunProtocolDATA(int *sockClient){
   int iStatus = 0;
   int iRestarting = 0;

   EWA_PROTOCOL ewaServerREPLY = {0};

   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD ewaClientDATACMD = {0};

   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER ewaClientDATAHEAD = {0}; 
   struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE *ewaClientDATA = NULL; /* Pointer because data is dynamic*/

   /* Receive DATACMD request. Since we check the header before this function
    * is invoked we don't have to do that here */
   if(recv(*sockClient, &ewaClientDATACMD, sizeof(EWA_PROTOCOL), 0) < 0){
      return -2;
   } 

   if(VerifyHeader(ewaClientDATACMD.stHead, 64, 1) <= 0){
      return 1;
   }

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
      return 1;
   }

   if(iValidFileExtension != 0){
      return 1;
   }

   /* Create filename at current directory (./<FILENAME>). Reusing buffer from validation step */
   char szFile[64] = "./"; 
   strcat(szFile, szFileNameBuffer);

   printf("DATACMD -> ACCEPT: %s\n", szFileNameBuffer);
   CreateServerReply(&ewaServerREPLY, MSG_READY, "FILENAME ACCEPT");

   /* DATACMD: Send reply  */
   if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
      return -1;
   }  

   int iTotalBytesRemaining;
   /* If there's no problems with initial header, start receiving data */

   /* Declaring variables for loop */
   int iBufferSize = 0;
   int iTotalBytesSaved = 0;
   int iEOF = 0;

   /*----DATAFILE RECEIVE LOOP----*/
   while(1){ 

      /* Exits the datafile protocol, returning the errorcode */
      if(iRestarting == 1 || iStatus != 0){
         CreateServerReply(&ewaServerREPLY, MSG_READY, "DATAFILE OK - WRITE COMPLETE");
         break;
      } 

      /* DATAFILE: If iEOF was hit we exit to start of CMD */
      if(iEOF == 1){
         iRestarting = 1;
         continue;
      } else {
         CreateServerReply(&ewaServerREPLY, MSG_ACCEPT, "DATA OK - READY");
      }

      /* DATAFILE: Send message with code defined above */
      if(send(*sockClient, &ewaServerREPLY, sizeof(ewaServerREPLY), 0) < 0){
         iStatus = -1;
         continue;
      }

      /* DATAFILE: PEEK new header */
      if(recv(*sockClient, &ewaClientDATAHEAD, sizeof(EWA_HEAD), MSG_PEEK) < 0){
         iStatus = -2;
         continue;
      } 

      /* DATAFILE: Verify and read byte count */
      int iRequestSize = VerifyHeader(ewaClientDATAHEAD, MAX_FILE, 1) + sizeof(EWA_HEAD);
      if(iRequestSize < 0){
         printf("REQUEST DATA CHECK FAILED: %d\n", iRequestSize);
         iStatus = 1;
         continue;
      }

      /* Size of data */
      int iTotalBytesRemaining = iRequestSize - sizeof(EWA_HEAD);

      /* DATAFILE: Allocates struct based on header size */
      ewaClientDATA = (struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATAFILE *) malloc(iRequestSize);
      if(ewaClientDATA == NULL){
         iStatus = -3;
         continue;
      }

      puts("HEADER VERIFIED - RECEIVING DATA");
      /* DATAFILE: Receives data struct based on header size */
      if(recv(*sockClient, ewaClientDATA, iRequestSize, MSG_WAITALL) < 0){
         free(ewaClientDATA);
         iStatus = -2;
         continue;
      } 

      printf("FILE IS %d BYTES\n", iTotalBytesRemaining);

      /* ----DATAFILE READ ENTRY----*/
      while(iTotalBytesRemaining > iTotalBytesSaved){

         /* Exits datafile read if error occured */
         if(iRestarting == 1 || iStatus != 0) break;

         /* If FileContent size is greater than MAX_READ, we read MAX_READ at a time */
         if(iTotalBytesRemaining > MAX_READ){
            iBufferSize = MAX_READ;

         /* Until the bytes to read is smaller than max read, where we read the remainig bytes */
         } else {
            iBufferSize = iTotalBytesRemaining;
         }

         printf("READING SEGMENT OF SIZE %d - %d REMAINING ...\n", iBufferSize, iTotalBytesRemaining);


         /* Allocate the buffer */
         char *pszBuffer = NULL;
         pszBuffer = (char *) malloc(iBufferSize + 1); /* One extra byte for null termination */
         if(pszBuffer == NULL){
            free(ewaClientDATA);
            ewaClientDATA = NULL;

            iStatus = -3;
            continue;
         }
         memset(pszBuffer, 0, iBufferSize);

         /* Targets to look for. If our "window" equals these targets then we 
	  * call EOF */
         char carrEOFTarget1[] = {'\n','\n','.','\n'};
         char carrEOFTarget2[] = {'\r','\n','\r','\n','.','\r','\n'};

	 /* Declaring window */
         char carrWindow[8];
         memset(carrWindow, '0', 8);

         /* End with a null terminator so for debugging */

         /* Checks entire buffer */
         int i, iCurrent;
         for(i = 0; i < iBufferSize; i++){
            iCurrent = iTotalBytesSaved + i;

            /* Create a 7 byte "window" at a time. That way we can check for the "\n.\n" exit pattern */
            /* Reads from right to left, starting at the third index.
            /* Shift every element down one index */
            int j;
            for(j = 0; j < 6; j++){
               carrWindow[j] = carrWindow[j + 1];
            }
            /* New element goes in rightmost index */
            carrWindow[6] = ewaClientDATA->acFileContent[iCurrent];

	    /* Prints the current window */
            printf("-> %d WINDOW=[ ", iCurrent);
            for(j = 0; j < 7; j++){
               if(carrWindow[j] == '\r')
                  printf("CR ");
               else if(carrWindow[j] == '\n')
                  printf("LF ");
               else 
                  printf("%c ", carrWindow[j]);
            }
            printf("] END\n");

            /* Checking the byte content */


            pszBuffer[i] = carrWindow[6];

            /* Checking for \n\n.\n */
            if(i >= 4 && memcmp(carrWindow, carrEOFTarget1, 4) == 0){
               printf("EOF(1) REACHED\n");
               iEOF = 1;
               break;
            }
            /* Checking for \r\n\r\n.\r\n */
            if(i >= 6 && memcmp(carrWindow, carrEOFTarget2, 7) == 0){
               printf("EOF(2) REACHED\n");
               iEOF = 1;
               break;
            }
         }

         /* Once we've checked the buffer, we write to the file
          * We reduce the read count for every time we write data to file */
         if(iBufferSize < MAX_READ){
            pszBuffer[iBufferSize] = '\0';
         }

         /* DATAFILE: Open the file in (a)ppend mode (new data is appended to the end of the file) */
         FILE *fpClientFile = fopen(szFile, "a");
         if(fpClientFile == NULL){
            free(ewaClientDATA);
            ewaClientDATA = NULL;
            iStatus = -4;
            continue;
         }

         /* Write the data from the buffer to the file */  
         iTotalBytesSaved += fprintf(fpClientFile, "%s", pszBuffer);
         /*fprintf(fpClientFile, pszBuffer); */

         /* If we hit EOF during the parse we exit the loop here. But first we let the client know we are done */
         if(iEOF == 1){
            /* Closing the file since EOF was reached */
            fclose(fpClientFile);
            fpClientFile == NULL;

            free(ewaClientDATA);
            free(pszBuffer);
            ewaClientDATA = NULL;

            /* EXIT DATAFILE READ */
            break;
         } 
      }/*-> DATAREAD LOOP*/
   }/*-> DATAFILE LOOP */

   return iStatus;
}
