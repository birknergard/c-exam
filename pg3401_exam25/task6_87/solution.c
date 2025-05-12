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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h> /*  */

#include <byteswap.h> /* Big to little endian conversion */

#if __BYTE_ORDER == __LITTLE_ENDIAN
uint64_t ntohll(uint64_t value) {
   return ((uint64_t)ntohl(value & 0xFFFFFFFF) << 32) | ntohl(value >> 32);
}

uint64_t htonll(uint64_t value) {
   return ((uint64_t)htonl(value & 0xFFFFFFFF) << 32) | htonl(value >> 32);
}
#else
#define htonll(x) (x)
#define ntohll(x) (x)
#endif

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

      printf("\nHEADER\n");
      for(i = 0; i < iHeaderLength; i++){
         if(reRequest.szHeader[i] == '\r') printf("\\r");
         else if(reRequest.szHeader[i] == '\n') printf("\\n");
         else printf("%c", reRequest.szHeader[i]);
      }
      puts("\nHEADER END\n");

      printf("\nENC AS HEX=\n");
      for(i = 0; i < iContentLength / 8; i++){
         /* Bytes are received in BIG ENDIAN */
         /* Ensure host byte order ? source: htons(3) man page */
         printf("IN %016llX ", reRequest.arrby8Encrypted[i].by8Base);
         //reRequest.arrby8Encrypted[i].by8Base = ntohll(reRequest.arrby8Encrypted[i].by8Base);
         //reRequest.arrby8Encrypted[i].by4[0] = ntohl(reRequest.arrby8Encrypted[i].by4[0]);
         //reRequest.arrby8Encrypted[i].by4[1] = ntohl(reRequest.arrby8Encrypted[i].by4[1]);
         /* Bytes are converted to LITTLE ENDIAN */
         printf("OUT %016llX \n", reRequest.arrby8Encrypted[i].by8Base);
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


      /* Writing contents of body to encrypted file (as signed char)*/
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
   int iIterations = 8;
   int iEndian = 1;

   /* Writing file to our buffer */
   fread(aun_by8Encrypted, sizeof(char), iFileContent, fpEncrypted);

   /* Closing the file */
   fclose(fpEncrypted);
   fpEncrypted = NULL;

   printf("\nENC (%d bytes) AS HEX, AFTER WRITE=\n", iFileContent);
   for(i = 0; i < iSize; i++){
      printf("%016llX ", aun_by8Encrypted[i].by8Base);
   }
   printf("\nENC AFTER WRITE END=\n");

   BYTE by4Key[4]; /* since every byte is the same, this is fine just for padding for one key modifier later */
   memset(by4Key, 0, 4);

   for(y = 1; y <= 32; y++){

      /* Doubles the iterations every other loop (MAX 64) */
      /*
      if(y % 2 != 0 ){
         if(y >= 3) iIterations += iIterations; 
      }
      */

      int i;
      for(i = 0; i <= 255; i++){

         /* Allocating for decipher container */
         char *szDeciphered = (char *) malloc(iSize + 1);
         if(szDeciphered == NULL){
            return -1;
         }
         memset(szDeciphered, 0, iSize);

         char cCheckedCharKey = (char) i;
         memset(by4Key, cCheckedCharKey, sizeof(unsigned char) * 4);

         /*
         for(a = 0; a < 16; a++){
            printf("%02X", by4Key[0]); 
         }
         puts("]");
         */

         int l = 0;
         /* Checks both in little and big endian */
         while(l < iSize){

            union UN_BY8 un_by8Encrypted;
            un_by8Encrypted.by8Base = aun_by8Encrypted[l].by8Base;

            un_by8Encrypted.by8Base = htonll(un_by8Encrypted.by8Base);
            /* Checks between little endian (host order) and big endian (network order) */
            if(y % 2 == 0){
               iEndian = 1;
               //un_by8Encrypted.by4[0] = htonl(un_by8Encrypted.by4[0]); 
            } else{
               /* Converts the padded byte to little endian */
               //printf("%016llX -- ", un_by8Encrypted.by8Base);
               iEndian = -1;
            } 

            /* Since one long long is equivalent to 2 * integers (4 bytes each) */
            /* Starting with a brute force approach. 
             * Checking all 255 ascii characters */

            /* Casting it to an unsigned long long(8 bytes) while receiving */

            /* For deciphering I tried to take the code from task4_threads.c and reverse it.
             * NOTE: As i probably mentioned in that source file, the code was essentially entirely based on
             * the supplied tea.c file and its documentation.

            /* Since it is a symmetrical algorithm that must mean that reversing it
             * would decipher, given the same key. */

            /* Since all the characters are supposedly the same, i can just insert my brute force
             * char in the deciphering algorithm. It only compared one char anyway so it shouldn't matter? 
             * 
             * In the original algorithm the same delta was
             * added to the sum 32 times. So perhaps we should try to have it start at 32 * delta? */
            iIterations = y;
            unsigned int uiDelta = 0x9E3779B9;
            unsigned int uiSum = (uiDelta * iIterations);

            int n;
            /* Deciphers padded byte */
            for(n = 0; n < iIterations; n++){
               /* Subtracting the amount of characters that was originally added "added".*/
               un_by8Encrypted.by4[1] -= (
                  (un_by8Encrypted.by4[0] << 4) + by4Key[2]) ^ 
                  (un_by8Encrypted.by4[0] + uiSum) ^ 
                  ((un_by8Encrypted.by4[0] >> 5) + by4Key[3]
               );

               un_by8Encrypted.by4[0] -= (
                  (un_by8Encrypted.by4[1] << 4) + by4Key[0]) ^ 
                  (un_by8Encrypted.by4[1] + uiSum) ^ 
                  ((un_by8Encrypted.by4[1] >> 5) + by4Key[1]
               );
               /* Since we  increased to sum to add to the encrypted character sum in the last algorithm,
                * we subtract it here */
               uiSum -= uiDelta;
            }
            if(uiSum != 0){
               printf("ERROR. exiting ...\n");
               return 1;
            }

            /* Converts the long (int, int) into a single char. When converting to char, C only keeps the first byte.
             * Since PKCS5 padding pads rightwards we know this is the intended char. */

            /* If every byte after the first == 0x07 the char is almost certainly decrypted*/

            /* Since the algorithm encrypts 32 bits of each paddes byte at the same time,
             * with the same encryption key, the two halves lowest or highest byte position according
             * to endianness will remain. Since the byte is padded with PKCS5 we know that one of the bytes is 
             * 0x07, and the other byte is the decrypted initial byte. Every other byte will have been scrambled
             * due to the algoritm ? */

            /* I figured this out by creating my own encrypted test with my solution from task 4. */
            if(iEndian == 1){
               if(un_by8Encrypted.schar[4] == 0x07){
                  //printf(" %02x ", un_by8Encrypted.schar[0]);
                  szDeciphered[l] = (char) un_by8Encrypted.schar[0];
                  iDeciphered = 1;
               } else {
                  iDeciphered = -1; 
               }

            } else {
               if(un_by8Encrypted.schar[3] == 0x07){
                  //printf(" %02x ", un_by8Encrypted.schar[7]);
                  szDeciphered[l] = (char) un_by8Encrypted.schar[7];
                  iDeciphered = 1;
               } else {
                  iDeciphered = -1; 
               }
            }

            l++;

         }

         if(iDeciphered == 1){
            printf("---RAN DECRYPT -> TEA ITERATIONS=%d, ENDIAN=%d, KEY=%02X\n", iIterations, iEndian, by4Key[0]);
            szDeciphered[iSize] = '\0';
            printf("Solution? %s\n", szDeciphered);
         }
      }
   }


   return 0;
}


