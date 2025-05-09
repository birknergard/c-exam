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
   if(iArgC < 4){
      perror("Insufficient arguments for client start.");
      return 1;
   }

   /* Parse program arguments 
      If flag was given, store id from arguments */
   int iPort;
   if(strcmp(arrpszArgV[1], "-port") == 0){
      iPort = atoi(arrpszArgV[2]);
   }

   char szServerID[50] = {0};  
   if(strcmp(arrpszArgV[3], "-id") == 0){
      memcpy(szServerID, arrpszArgV[4], 49);
   }
   szServerID[49] = '\0';

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
   int iLengthStart = strcspn(szResponseHeader, "\nContent-Length: ");
   if(iLengthStart == strlen(szResponseHeader)){
      printf("Invalid header received. Closing ...\n");
      close(sockClient); 
      sockClient = -1;
      return 1;

   }

   /* Finding end index of ContentLength */
   int iLengthEnd = strcspn(szResponseHeader + iLengthStart, "\n");

   int iDigits = iLengthEnd - iLengthStart;
   int iContentLength = -1, i, n;
   char cByte;

   for(i = iLengthStart; i <= iLengthEnd; i++){
      cByte = szResponseHeader[i];
      /* Checks if byte is not digit */
      if(47 > cByte && cByte > 58){
         printf("ContentLength includes non digit value. Closing ...\n");
         return 1;
      } 

      int iByte = (int) cByte;
      /* Multiply itby the power of the 10^digitplace */
      for(n = 0; n <= iDigits; i++){
         iByte = iByte * 10;
      }
      printf("iByte = %d", iByte);
      iContentLength = iByte;

      iDigits--;
   }
   printf("ContentLength from header %d", iContentLength);


   /* Debugging, attempting to print response */
   printf("RESPONSE=%s\n", szResponseHeader);
   close(sockClient); sockClient = -1;

   return 0;
}

