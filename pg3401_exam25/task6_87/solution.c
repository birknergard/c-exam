/*
 * TITLE: Task 6 Solution
 * AUTHOR: 87
 * DESCRIPTION
 * */
#include "solution.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int isReadableAscii(char c){
   if((127 > c && c > 31) || c == '\n' || c == '\r' || c == '\0' || c == '\t'){
      return 0;
   }
   return 1;
}

/* NOTE: As i probably mentioned in that source file, the code was essentially entirely based on
the supplied tea.c file and its documentation. */
void decipher(unsigned int *const v, unsigned int *const w, unsigned int *k, unsigned int n){
   register unsigned int y=v[0], z=v[1], delta=0x9E3779B9, sum=delta * n;
   register unsigned int a=k[0], b=k[1], c=k[2], d=k[3];

   int i; 
   for(i = 0; i < n; i++){
      z -= (y << 4) + c ^ (y+sum) ^ (y>>5) + d;
      y -= (z << 4) + a ^ (z+sum) ^ (z>>5) + b;
      sum -= delta;
   }

   w[0] = y, w[1] = z;
}

int main(int iArgC, char **arrpszArgV){

   if(iArgC < 2){
      printf("Need at least one argument. Exiting ...\n");
      return 1;
   } 

   /*-----------------PART 1: CLIENT-------------------*/
   /* Checking if arguments were provided */
   if(strcmp(arrpszArgV[1], "-id") == 0){
      /* Parse program arguments 
         If flag was given, store id from arguments */
      char szServerID[50] = {0};  
      memcpy(szServerID, arrpszArgV[2], 49);
      szServerID[49] = '\0';

      int iPort;
      if(strcmp(arrpszArgV[3], "-port") == 0){
         iPort = atoi(arrpszArgV[4]);
      }

      
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
         printf("Connection failed with error %d. Exiting ...\n", errno);
         close(sockClient); sockClient = -1;
         return -1;
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
      if((iReceived = recv(sockClient, szResponseHeader, 900, MSG_PEEK)) < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 

      /* Find start of ContentLength */
      char szReader[258] = {0};
      char c;
      int i, k;

      /* This loop finds the range from the first newline it detects and the following one. */
      int iHeaderLength = 1;
      int iarrLine[2], iFoundSizeIndexes = -1;
      memset(iarrLine, -1, sizeof(int) * 2);

      /* I devised this method to "safely" read any http header and find content-length */
      for(i = 1; i < MAX_HTTP_HEADER; i++){
         /* When found \r\n\r\n, we have reached the end of the header */
         if(
            i > 4 &&
            szResponseHeader[i - 3] == '\r' && szResponseHeader[i - 2] == '\n' &&
            szResponseHeader[i - 1] == '\r' && szResponseHeader[i] == '\n'
         ) {
            iHeaderLength = i + 1;
            printf("LENGTH OF HEADER %d", iHeaderLength);
            break;
         }

         if(iFoundSizeIndexes != 1){
            if(szResponseHeader[i - 1] == '\r' && szResponseHeader[i] == '\n'){
               iarrLine[0] = i + 1; /* Omitting the \r character */

               k = 0;
               while(k < strlen(szResponseHeader)){
                  szReader[k] = szResponseHeader[iarrLine[0] + k];

                  if(szResponseHeader[iarrLine[0] + k - 1] == '\r' && szResponseHeader[iarrLine[0] + k] == '\n'){
                     iarrLine[1] = k + iarrLine[0] - 2; /* Omitting the \r\n characters */
                     szReader[k] = '\0';
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
                  iFoundSizeIndexes = 1;
               }
            }
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
            printf("ContentLength includes non digit value. Exiting ...\n");
            close(sockClient); sockClient = -1;
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

      if(iContentLength > MAX_HTTP_RESPONSE){
         printf("Content length of response exceeds max. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return 1;
      }


      /*-------------RECIEVING FILE--------------*/
      printf("Content length %d\n", iContentLength);

      /* Allocating struct to hold response. Divided into the header (string array) and encrypted message (long array) */
      ENC_REQUEST reRequest;
      reRequest.szHeader = NULL;
      reRequest.arrby8Encrypted = NULL;

      reRequest.szHeader = (unsigned char *) malloc(iHeaderLength);
      if(reRequest.szHeader == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      reRequest.arrby8Encrypted = (union UN_BY8 *) malloc(sizeof(BY8) * (iContentLength / 8));
      if(reRequest.arrby8Encrypted == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      /* Receive header first */
      iReceived = recv(sockClient, reRequest.szHeader, iHeaderLength, 0);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 

      /* Then receive the encrypted message in 64 bit array first */
      iReceived = recv(sockClient, reRequest.arrby8Encrypted, iContentLength, 0);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 

      printf("\nHEADER=");
      for(i = 0; i < iHeaderLength; i++){
         if(reRequest.szHeader[i] == '\r') printf("\\r");
         else if(reRequest.szHeader[i] == '\n') printf("\\n");
         else printf("%c", reRequest.szHeader[i]);
      }
      puts("HEADER END\n");

      printf("\nENC AS c=\n");
      for(i = 0; i < iContentLength / 8; i++){
         printf("IN %016llX ", reRequest.arrby8Encrypted[i].by8Base);
         /* Bytes are converted to LITTLE ENDIAN */
         //reRequest.arrby8Encrypted[i].by4[0] = ntohl(reRequest.arrby8Encrypted[i].by4[0]);
         //reRequest.arrby8Encrypted[i].by4[1] = ntohl(reRequest.arrby8Encrypted[i].by4[1]);
      }
      printf("\nENC END=\n");

      /* Opening / creating file handle */
      FILE *fpEncryptedFile = NULL;
      fpEncryptedFile = fopen("./encrypted.bin", "wb");
      if(fpEncryptedFile == NULL){
         free(reRequest.szHeader);
         reRequest.szHeader = NULL;
         free(reRequest.arrby8Encrypted);
         reRequest.arrby8Encrypted = NULL;
         close(sockClient); sockClient = -1;
         return -1;
      }

      printf("\nENC (%d bytes) AS HEX, AFTER WRITE=\n", iContentLength);
      for(i = 0; i < iContentLength / 8 ; i++){
         printf("%016llX ", reRequest.arrby8Encrypted[i].by8Base);
      }
      printf("\nENC AFTER WRITE END=\n");

      /* Writing contents of body to encrypted file 8 byte snippets (each encrypted character)*/
      fwrite(reRequest.arrby8Encrypted, sizeof(long), iContentLength / 8, fpEncryptedFile);

      /* Closing file and cleaning up */
      fclose(fpEncryptedFile);
      fpEncryptedFile = NULL;

      free(reRequest.szHeader);
      reRequest.szHeader = NULL;
      free(reRequest.arrby8Encrypted);
      reRequest.arrby8Encrypted = NULL;
      close(sockClient);
      
      sockClient = -1;
   }


   /*-------------PART 2: DECRYPTING THE FILE---------------------*/

   
   /* Preparing pointers */
   FILE *fpEncrypted = NULL;

   /* Opening file */
   //fpEncrypted = fopen("../task4_87/task4_pg2265.bin", "rb"); 
   fpEncrypted = fopen("encrypted.bin", "rb");
   if(fpEncrypted == NULL){
      printf("Failed to open file.\n");
      return 1;
   }

   /* Getting file size */
   int iFileContent;
   if(fseek(fpEncrypted, 0, SEEK_END) == 0){
      iFileContent = ftell(fpEncrypted);
      rewind(fpEncrypted);
   }

   int iSize = iFileContent / 8; /* Original file was encrypted with 64 bit padding, meaning the file is actually 648 / 8 = 81 */
   char *szDeciphered = NULL;

   /* For more flexibility */
   union UN_BY8 aun_by8Encrypted[81] = {0};

   BYTE byDeciphered;

   int iReadCharacters;
   int i, y, q = 0;
   unsigned char cKeyChar = 0;
   int iDeciphered = 0;
   int iIterations = 32;
   int iEndian = 1;

   /* Writing file to our buffer */
   fread(aun_by8Encrypted, sizeof(BY8), iFileContent / 8, fpEncrypted);

   /* Closing the file */
   fclose(fpEncrypted);
   fpEncrypted = NULL;

   printf("\nENC (%d bytes) AS HEX, AFTER WRITE=\n", iFileContent);
   for(i = 0; i < iSize; i++){
      printf("%016llX ", aun_by8Encrypted[i].by8Base);
   }
   printf("\nENC AFTER WRITE END=\n");


   for(i = 0; i <= 255; i++){

      /* Allocating for decipher container */
      char *szDeciphered = (char *) malloc(iSize + 1);
      if(szDeciphered == NULL){
         return -1;
      }
      memset(szDeciphered, 0, iSize);

      char cCheckedCharKey = (unsigned char) i;

      /* since every byte is the same, this is fine just for padding for one key modifier later */
      union UN_BY16 by16Key; 
      memset(by16Key.by, (BYTE) cCheckedCharKey, 16);
      
      union UN_BY8 un_by8Buffer[81];

      int l = 0;
      /* Checks both in little and big endian */
      while(l < iSize){

         union UN_BY8 un_by8Deciphered;
         un_by8Deciphered.by8Base = 0;

         //printf("BEFORE %08lx == %08lx\n", aun_by8Encrypted[l].by8Base, un_by8Deciphered.by8Base);
         decipher(aun_by8Encrypted[l].by4, un_by8Deciphered.by4, by16Key.by4, iIterations);

         /* DEBUG: Making sure the decipher actually change the value */
         //printf("AFTER %08lx == %08lx\n", aun_by8Encrypted[l].by8Base, un_by8Deciphered.by8Base);

         un_by8Buffer[l].by8Base = un_by8Deciphered.by8Base;
         l++;

      }

      if(cCheckedCharKey == 12){
         int k;
         printf("Partially decrypted ->\n");
         for(k = 0; k < iSize; k++){
            printf("0x%016llX ", un_by8Buffer[k].by8Base);
         }
      }



      //szDeciphered[l] = (char) un_by8Deciphered.by[0];
      /*
      int iChars, iFailed;
      for(iChars = 0; iChars < iSize; iChars++){
         iFailed = isReadableAscii(szDeciphered[iChars]);
         if(iFailed == 1){
            break;
         }
      }

      if(iFailed != 1){
         printf("---RAN DECRYPT -> TEA ITERATIONS=%d, ENDIAN=%d, KEY=%02x\n", iIterations, iEndian, cCheckedCharKey);
         szDeciphered[iSize] = '\0';
         printf("\nSolution? %s\n\n", szDeciphered);
         int k;
         for(k = 0; k < iSize; k++){
            printf("%c", (char) szDeciphered[k]);
         }
      }
      */
   }

   return 0;
}


