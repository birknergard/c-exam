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

#include <byteswap.h> /* Big to little endian conversion */
#include <endian.h>

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

      /* Allocating string to hold response. Add one extra to ensure null termination */
      unsigned char *szResponse = NULL;
      szResponse = (unsigned char *) malloc(iHeaderLength + iContentLength + 1);
      if(szResponse == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      /* Receive file */
      iReceived = recv(sockClient, szResponse, (iHeaderLength + iContentLength), 0);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 


      printf("\nRESPONSE AS HEX=\n");
      for(i = iHeaderLength; i < iContentLength; i++){
         printf("%02x ", szResponse[i]);
      }

      /* Opening / creating file handle */
      FILE *fpEncryptedFile = NULL;
      fpEncryptedFile = fopen("./encrypted.bin", "wb");
      if(fpEncryptedFile == NULL){
         free(szResponse);
         szResponse = NULL;
         close(sockClient); sockClient = -1;
         return -1;
      }

      /* Writing contents of body to encrypted file*/
      fwrite((szResponse + iHeaderLength), sizeof(unsigned char), iContentLength, fpEncryptedFile);

      /* Closing file and cleaning up */
      fclose(fpEncryptedFile);
      fpEncryptedFile = NULL;
      free(szResponse);
      szResponse = NULL;
      close(sockClient);
      sockClient = -1;
   }


   /*-------------PART 2: DECRYPTING THE FILE---------------------*/

   /* For testing. when i merge the files togheter */
   
   int iFileContent = 648;
   int iSize = iFileContent / 8; /* Original file was encrypted with 64 bit padding, meaning the file is actually 648 / 8 = 81 */

   /* Preparing pointers */
   FILE *fpEncrypted = NULL;
   char szDeciphered[82] = {0};

   /* Opening file */
   fpEncrypted = fopen("encrypted.bin", "rb");
   if(fpEncrypted == NULL){
      printf("Failed to open file.\n");
      return 1;
   }
   
   /* For more flexibility */
   BY4 byEncrypted[2] = {0};
   BY4 by4Key[4];

   BYTE byDeciphered;
   int iReadCharacters;
   int i, y, q, iKeyChar;
   int iDeciphered = 0;
   int iIterations = 8;

   for(y = 1; y <= 4; y++){
      rewind(fpEncrypted);
      printf("With %d iterations on tea.\n", iIterations);
      int s = 2;
      while(++s < y) iIterations += iIterations;

      for(iKeyChar = 0; iKeyChar <= 255; iKeyChar++){
         rewind(fpEncrypted);

         int l = 0;
         /* Checks both in little and big endian */
         while(l < iSize){
            iReadCharacters = (int) fread(byEncrypted, sizeof(BY4), 2, fpEncrypted);
            if(iReadCharacters != 2){
                printf("Read failed: iReadCharacters = %d\n", iReadCharacters);
                printf("feof: %d, ferror: %d, errno: %d (%s)\n",
                       feof(fpEncrypted), ferror(fpEncrypted), errno, strerror(errno));
                break;
            }
            /*printf("Read block: [%02x, %02x] - ", byEncrypted[0], byEncrypted[1]); */

            /* Since one long long is equivalent to 2 * integers (4 bytes each) */
            /* Starting with a brute force approach. 
             * Checking all 255 ascii characters */

            /* Casting it to an unsigned long long(8 bytes) while receiving */

            /* For deciphering I tried to take the code from task4_threads.c and reverse it.
             * NOTE: As i probably mentioned in that source file, the code was essentially entirely based on
             * the supplied tea.c file.

            /* Since it is a symmetrical algorithm that must mean that reversing it
             * would decipher, given the same key. */

            /* Since all the characters are supposedly the same, i can just insert my brute force
             * char in the deciphering algorithm. It only compared one char anyway so it shouldn't matter? 
             * 
             * In the original algorithm the same delta was
             * added to the sum 32 times. So perhaps we should try to have it start at 32 * delta? */
            /*NOTE: I think the delta is the problem, or the key(doubtful)*/
            unsigned long uiSum = (0x9E3779B9 * iIterations), uiDelta = 0x9E3779B9;

            int n;
            /* Deciphers padded byte */
            for(n = 0; n < iIterations; n++){
               /* Subtracting the amount of characters that was originally added "added".*/
               byEncrypted[1] -= (byEncrypted[0] << 4) + iKeyChar ^ byEncrypted[0] + uiSum ^ ( byEncrypted[0] >> 5) + iKeyChar;
               byEncrypted[0] -= (byEncrypted[1] << 4) + iKeyChar ^ byEncrypted[1] + uiSum ^ ( byEncrypted[1] >> 5) + iKeyChar;
               /* Since we  increased to sum to add to the encrypted character sum in the last algorithm,
                * we subtract it here */
               uiSum -= uiDelta;
            }

            byDeciphered = (char) byEncrypted[0];

            szDeciphered[l] = byDeciphered;
            l++;
         }

         szDeciphered[iSize] = '\0';

         /* Check that all characters are usable in plaintext
          * For a char to be readable it has have a value between 32 and 127 (ascii printable characters). So we check every char
          * in the array if they are within this range. If they are, we print to terminal for now to see if it makes sense.*/
         iDeciphered = 1;
         for(i = 0; i < iSize; i++){
            /* If the inverse condition happens, we restart */
            if(127 < szDeciphered[i]){
               iDeciphered = 0;
            }
         }
         if(iDeciphered == 1){
            printf("\nDECIPHERED TEXT! TERATIONS:%d ?\n%s", iIterations, szDeciphered);
            break;
         } else {
            printf("\nDECIPHER FAILED\n");
         }
      }
   }

   fclose(fpEncrypted);
   fpEncrypted = NULL;

   return 0;
}

