/*
 * TITLE: Task 6 Solution
 * AUTHOR: 87
 * DESCRIPTION
 *
 *
 * */
#include "solution.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h> /*  */

int main(int iArgC, char **arrpszArgV){
   /* Checking if arguments were provided */
   if(iArgC < 5){
      puts("Insufficient arguments for client start.");
      return 1;
   }

   /* Parse program arguments 
      If flag was given, store id from arguments */
   char szServerID[50] = {0};  
   if(strcmp(arrpszArgV[1], "-id") == 0){
      memcpy(szServerID, arrpszArgV[2], 49);
   }
   szServerID[49] = '\0';

   int iPort;
   if(strcmp(arrpszArgV[3], "-port") == 0){
      iPort = atoi(arrpszArgV[4]);
   }

   /*--------CLIENT SETUP-----------*/
   
   printf("CLIENT: Started -port %d -id %s\n", iPort, szServerID);
   /* Contains client status */
   int iStatus = 0;

   /* Client variables */
   int sockClient = -1;
   struct sockaddr_in saAddress;

   /* Set client socket */
   sockClient = socket(AF_INET, SOCK_STREAM, 0);
   if(sockClient < 0){
      printf("Socket failed to open: CODE %d\n", errno);
      return -1;
   }

   saAddress.sin_family = AF_INET;
   saAddress.sin_port = htons(iPort);
   saAddress.sin_addr.s_addr = htonl(0x7F000001);

   iStatus = connect(sockClient, (struct sockaddr *) &saAddress, sizeof(saAddress)); 
   if(iStatus < 0){
      printf("Connection failed: CODE %d\n", errno);
      close(sockClient); sockClient = -1;
      return -1;
   } 

   if(iStatus != 0){
      printf("Client could not be started\n");
      return 1;
   }

   /*--------RECEIVE FILE-----------*/

   /* Communication status */
   int iSent;
   int iReceived;

   /* File handle */
   FILE *fpEncrypted = NULL;

   /* Protocol container */
   char szResponseHeader[MAX_HTTP_HEADER + 1] = {0};

   /* Check header */
   if((iReceived = recv(sockClient, szResponseHeader, MAX_HTTP_HEADER, MSG_PEEK)) < 0){
      printf("Failed to receive response from server - errcode %d\n", errno);
      close(sockClient); sockClient = -1;
      return -2; 
   } 
   szResponseHeader[MAX_HTTP_HEADER] = '\0';

   /* Find start of ContentLength */
   printf("Finding start of ContentLength\n");
   char szReader[258] = {0};
   char c;
   int i, k;

   /* This loop finds the range from the first newline it detects and the following one. */
   int iarrLine[2], iFoundFirst = -1;
   memset(iarrLine, -1, sizeof(int) * 2);

   /* I devised this method to "safely" read any http header and find content-length */
   for(i = 1; i < strlen(szResponseHeader); i++){
      if(szResponseHeader[i - 1] == '\r' && szResponseHeader[i] == '\n'){
         iarrLine[0] = i + 1;
         k = 0;
         while(k < strlen(szResponseHeader)){
            szReader[k] = szResponseHeader[iarrLine[0] + k];
            if(szResponseHeader[iarrLine[0] + k - 1] == '\r' && szResponseHeader[iarrLine[0] + k] == '\n'){
               iarrLine[1] = k + iarrLine[0] - 2; /* Omitting the \r\n characters */
               szReader[k] = '\0';
               printf("\nLINE %s\n", szReader);
               break;
            }
            k++;
         }

         /* Check line created for match with "Content-Length: " */
         if(strncmp(szReader, "Content-Length: ", 15) == 0){
            printf("Content-Length found!\n");
            
            /* If found, we store the location of where the number would start " ". 
             * Where the number will end is already stored in the second index*/
            iarrLine[0] += 16;
            break;
         }

         memset(szReader, 0, 258);
         i += k;
      }
   }

   /* Finding end index of ContentLength */
   /*
   printf("Finding end of ContentLength\n");
   iLengthEnd += strcspn((szResponseHeader + iLengthStart), "\r\n");
   */
   int iDigits = iarrLine[1] - iarrLine[0];
   int iContentLength = 0, n;
   char cByte, iByte;

   /* Calculate size based on every digit in the range */
   for(i = iarrLine[0]; i <= iarrLine[1]; i++){
      cByte = szResponseHeader[i];
      /* Checks if byte is not digit */
      if(47 > cByte && cByte > 58){
         printf("ContentLength includes non digit value. Closing ...\n");
         return 1;
      } 

      int iByte = (int) (cByte - 48);
      /* set digit to the power of the 10^digitplace */
      for(n = iDigits; 0 < n; n--){
         iByte = iByte * 10;
      }
      iContentLength += iByte;

      iDigits--;
   }

   if(iContentLength < MAX_HTTP_RESPONSE){
      printf("Content length of response exceeds max. Exiting ...\n");
      return 1;
   }


   /*-------------RECIEVING FILE--------------*/
   /* Receive file */
   if((iReceived = recv(sockClient, szResponseHeader, MAX_HTTP_HEADER, MSG_PEEK)) < 0){
      printf("Failed to receive response from server - errcode %d\n", errno);
      close(sockClient); sockClient = -1;
      return -2; 
   } 
   szResponseHeader[MAX_HTTP_HEADER] = '\0';



   /* Debugging, attempting to print response */
   close(sockClient); sockClient = -1;

   return 0;
}

