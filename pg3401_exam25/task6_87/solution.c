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

/*
 * TODO: Make this inline instead
 * Function to check whether the given character is a readable ascii character.
 * Used as a whitelist.
 * */
int isNonReadableAscii(BYTE c){
   /* Checks every readable ascii character, as well as CR and LF */

   if(c == '\n') return 0;
   if(c == '\r') return 0;
   if(c == 0x07) return 0;

   if(0x19 > c) return 1;
   if(0x7F < c) return 1;
   else return 0;
}

int main(int iArgC, char **arrpszArgV){

   if(iArgC < 2){
      printf("Need at least one argument. Exiting ...\n");
      return 1;
   } 

   /*-----------------PART 1: CLIENT--------------------------*/
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


      /*--------VERIFYING HTTP DATA-----------*/

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

      int iDigits = iarrLine[1] - iarrLine[0];
      int iContentLength = 0, n;
      char cByte, iByte;

      /* Calculate size based on every digit in the range */
      for(i = iarrLine[0]; i <= iarrLine[1]; i++){
         cByte = szResponseHeader[i];
         /* Checks if byte is not digit */
         if(47 > cByte && cByte > 58){
            printf("ContentLength includes non digit value. Exiting ...\n");
            close(sockClient);
            sockClient = -1;
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
         close(sockClient);
         sockClient = -1;
         return 1;
      }

      if(iContentLength % 8 != 0){
         printf("Size is not based on 8 byte padded structures.");
         close(sockClient);
         sockClient = -1;
         return 1;
      }

      /*-------------RECEIVING FILE--------------*/
      printf("Content length %d\n", iContentLength);

      /* Allocating struct to hold response. Divided into the header (string array) and encrypted message (long array) */
      ENC_REQUEST erFullData = {0};
      erFullData.szFull = NULL;
      /*erFullData.arrby8Encrypted = NULL;
      */

      erFullData.szFull = (char *) malloc(iHeaderLength + iContentLength);
      if(erFullData.szFull == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }
      /*

      erFullData.arrby8Encrypted = (union UN_BY8 *) malloc(iContentLength);
      if(erFullData.arrby8Encrypted == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }
      */

      /* NOTE: TESTING RECEIVING EVERYTHING AT ONCE /*
      /* Receive header first 
      iReceived = recv(sockClient, szHeader, iHeaderLength, 0);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      }
      */

      /* NOTE: RECEIVING EVERYTHING */
      /* Then receive the encrypted message in 64 bit array first */
      iReceived = recv(sockClient, erFullData.szFull, iContentLength + iHeaderLength, 0);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 

      printf("\nHEADER=");
      for(i = 0; i < iHeaderLength; i++){
         if(erFullData.szFull[i] == '\r') printf("\\r");
         else if(erFullData.szFull[i] == '\n') printf("\\n");
         else printf("%c", erFullData.szFull[i]);
      }
      puts("HEADER END\n");

      /* Opening / creating file handle */
      FILE *fpEncryptedFile = NULL;
      fpEncryptedFile = fopen("encrypted.bin", "wb");
      if(fpEncryptedFile == NULL){
         free(erFullData.szFull);
         erFullData.szFull = NULL;
         //free(erFullData.arrby8Encrypted);
         //erFullData.arrby8Encrypted = NULL;
         close(sockClient); sockClient = -1;
         return -1;
      }

      printf("2.\n");
      printf("\nENC AS c=\n");
      for(i = iHeaderLength; i < iContentLength + iHeaderLength; i++){
         BYTE byCurrent = erFullData.szFull[i];
         printf("0x%02x ", byCurrent);
         fwrite(&byCurrent, sizeof(BYTE), 1, fpEncryptedFile);
         //printf("0x%016llX ", erFullData.arrby8Encrypted[i].by8Base);
         /* Bytes are converted to LITTLE ENDIAN */
         //reRequest.arrby8Encrypted[i].by4[0] = ntohl(reRequest.arrby8Encrypted[i].by4[0]);
         //reRequest.arrby8Encrypted[i].by4[1] = ntohl(reRequest.arrby8Encrypted[i].by4[1]);
      }
      printf("\nENC END=\n");

      printf("3.\n");

      /*
      printf("4.\n");
      printf("\nENC (%d bytes) AS HEX, AFTER WRITE=\n", iContentLength);
      for(i = 0; i < iContentLength / 8 ; i++){
         printf("%016llX ", erFullData.arrby8Encrypted[i].by8Base);
      }
      printf("\nENC AFTER WRITE END=\n");
      */

      printf("5.\n");
      /* Writing contents of body to encrypted file in 8 byte segments (each encrypted character)*/

      printf("6.\n");
      /* Closing file and cleaning up */
      fclose(fpEncryptedFile);
      fpEncryptedFile = NULL;

      free(erFullData.szFull);
      erFullData.szFull = NULL;
      //free(erFullData.arrby8Encrypted);
      //erFullData.arrby8Encrypted = NULL;
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

   /* Stored as a union array for more flexibility */
   union UN_BY8 *aun_by8Encrypted = NULL;
   aun_by8Encrypted = (union UN_BY8 *) malloc(iFileContent);
   if(aun_by8Encrypted == NULL){
      fclose(fpEncrypted);
      fpEncrypted = NULL;
      return 1;
   }

   /* Original file was encrypted with 64 bit padding, meaning the file is actually 648 / 8 = 81 */
   int iSize = iFileContent / 8; 

   /* Writing file to our buffer */
   fread(aun_by8Encrypted, sizeof(BY8), iSize, fpEncrypted);

   /* Closing the file */
   fclose(fpEncrypted);
   fpEncrypted = NULL;

   int iIterations = 32;

   /* Change this to toggle between checking one, or all the bytes */
   int iCheckAll = 0;

   unsigned char cKeyChar = 0;

   printf("\nENC (%d bytes, %d sz) AS HEX, AFTER WRITE=\n", iFileContent, iSize);
   int i;
   for(i = 0; i < iSize; i++){
      printf("%016llX ", aun_by8Encrypted[i].by8Base);
   }
   printf("\nENC AFTER WRITE END=\n");

   char *szDeciphered = NULL;

   /* Runs the decryption for extra iterations.
    * 0. No endian conversion
    * */
   int t;
   for(t = 0; t <= 1; t++){

      int iFailed = 0;
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
         memset(by16Key.by4, cCheckedCharKey, sizeof(unsigned int) * 4);

         //printf("\nThe key value: 0x%08lx\n", by16Key.by4[0]);

         /* Stored as a union array for more flexibility */
         union UN_BY8 *aun_by8Buffer = NULL;
         aun_by8Buffer = (union UN_BY8 *) malloc(iFileContent);
         if(aun_by8Buffer == NULL){
            free(aun_by8Encrypted);
            aun_by8Encrypted = NULL;
            return 1;
         }

         /* Loop until every padded byte has been checked */
         int l = 0;


         while(l < iSize){

            /* Structure to hold the decrypted byte */
            union UN_BY8 un_by8Deciphered;
            un_by8Deciphered.by4[0] = 0;
            un_by8Deciphered.by4[1] = 0;
            /* Also attemted to convert to little endian, but result scrambled identically.
             * Converting before decryption also led to no valid results. */
            //if(l == 0){ printf("1. %08X %08X\n",un_by8Deciphered.by4[0],un_by8Deciphered.by4[1]); }
            /*
            if(t == 1){
               aun_by8Encrypted[l].by4[0] = ntohl(aun_by8Encrypted[l].by4[0]);
               aun_by8Encrypted[l].by4[1] = ntohl(aun_by8Encrypted[l].by4[1]);
            }
            */
            /* [SUPPOSED TO] decipher the given long by split integer (4,4). The key is always the same
             * integer for this task since every byte in the key is the same. Decrypted long is stored in
             * the Deciphered union.*/

            /* Running algorithm (Made by David Wheeler and Roger Needham, provided by EWA) */
            register unsigned int uiSum = 0xC6EF3720, uiDelta = 0x9E3779B9;
            register unsigned int uiKeyOne = by16Key.by4[0], uiKeyTwo = by16Key.by4[1], uiKeyThree = by16Key.by4[2], uiKeyFour = by16Key.by4[3];
            register int n = 32;

            /* Store each separate int in a buffer */
            register unsigned int byY = aun_by8Encrypted[l].by4[0];
            register unsigned int byZ = aun_by8Encrypted[l].by4[1];

            /* Encrypt each half of the padded byte */
            while(n-->0){
               byZ -= (byY << 4) + uiKeyThree ^ byY + uiSum ^ ( byY >> 5) + uiKeyFour;
               byY -= (byZ << 4) + uiKeyOne ^ byZ + uiSum ^ ( byZ >> 5) + uiKeyTwo;
               uiSum -= uiDelta;
            }

            /* Store the encrypted byte back into their original indices */
            un_by8Deciphered.by4[0] = byY;
            un_by8Deciphered.by4[1] = byZ;

            /* Also attemted to convert to little endian, but result scrambled identically.
             * Converting before decryption also led to no valid results. */
            /*
            if(t == 2){
               aun_by8Buffer[l].by4[0] = ntohl(un_by8Deciphered.by4[0]);
               aun_by8Buffer[l].by4[1] = ntohl(un_by8Deciphered.by4[1]);
            }
            */
            //if(l == 0){ printf("2. %08X %08X\n",un_by8Deciphered.by4[0],un_by8Deciphered.by4[1]); }
            /* Checks the first char, given padding goes 0xVA 0x07 0x07 0x07 0x07 0x07 0x07 0x07 
             * This could change as a result of endianness */

            /* If i only want to check 1 padded byte, go here */
            if(iCheckAll == 1){
               aun_by8Buffer[l].by4[0] = un_by8Deciphered.by4[0]; 
               aun_by8Buffer[l].by4[1] = un_by8Deciphered.by4[1]; 

               /* Check every single byte in the long for valid bytes and print if true */
               int u;
               for(u = 0; u <= 7; u++){
                  /* If we found non readable ascii we break the loop */
                  if(isNonReadableAscii((BYTE) aun_by8Buffer[l].by[u]) != 0){
                     break;
                  }
                  /* If we got here, that means the long passed the test */
                  if(u == 7){
                     printf("\nEncrypted Byte, on key=%d :0x%016llx\n", i, aun_by8Encrypted[l].by8Base);
                     printf("Attempted decipher, on key=%d :0x%016llx\n\n", i, aun_by8Buffer[l].by8Base);
                  }
               }
               break;

            } else {
               /* Store the decyphered byte in its correlating position in the buffer */
               aun_by8Buffer[l].by4[0] = un_by8Deciphered.by4[0]; 
               aun_by8Buffer[l].by4[1] = un_by8Deciphered.by4[1]; 

               /* Which specific byte to store */
               int iByteToSave = 0;

               if(isNonReadableAscii((BYTE) aun_by8Buffer[l].by[iByteToSave]) != 0){
                  break;
               }

               /* Stores the decrypyted byte (VERIFY WHOLE 64 BYTE FIRST, DO THIS LATER)*/
               szDeciphered[l] = (char) aun_by8Buffer[l].by[iByteToSave];

               /* Increments l, check next padded byte */
               l++;
            }
         }

         if(iCheckAll != 1){


            //printf("\nDEC (%d bytes, KEY=%d ) AS HEX, AFTER WRITE=\n", iFileContent, i);
            //printf("%016llX ", aun_by8Buffer[0].by8Base);
            //printf("\nDEC AFTER WRITE END=\n");

            szDeciphered[l] = '\0';

            if(l == iSize){
               /* Verifies iterator l */
               printf("Decyphered %d characters\n", l);
               printf("\n\n---RAN DECRYPT -> TEA ITERATIONS=%d, ENDIAN=%d, KEY=%02x\n", iIterations, t, cCheckedCharKey);
               printf("\n%s\n\n", szDeciphered);
            }
         }

         free(szDeciphered);
         szDeciphered = NULL;

         free(aun_by8Buffer);
         aun_by8Buffer = NULL;
      }
      if(iFailed == 1) printf("ITER %d: Failed to decypher\n", t);
   }
   free(aun_by8Encrypted);

   return 0;
}


