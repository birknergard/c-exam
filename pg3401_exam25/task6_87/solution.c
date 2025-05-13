/*
 * TITLE: Task 6 Solution
 * AUTHOR: 87
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
 * Function to check whether the given character is a readable ascii character.
 * Used as a whitelist.
 * */
int isNonReadableAscii(BYTE c){
   /* Checks every readable ascii character, as well as CR and LF */

   if(c == '\n') return 0;
   if(c == '\r') return 0;
   /* For padded byte */
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
   //if(strcmp(arrpszArgV[1], "-id") == 0){
   /* Parse program arguments if flag was given, store id from arguments */
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
   
   int iContentLength = 0, n;
   union UN_BY8 *aun_by8Encrypted = NULL;
   int iReceived;

   /* If port is 7575 we receive the testfile "selfmade.enc into the buffer. 
    * It is tested but not saved." */
   if(iPort == 7575){
      /* Allocating for testfile */
      aun_by8Encrypted = (union UN_BY8 *) malloc(720);
      if(aun_by8Encrypted == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      /* Receives testfile*/      
      iReceived = recv(sockClient, aun_by8Encrypted, 720, 0);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 

      /* The testfile is always 720 bytes*/
      iContentLength = 720;


   /* Otherwise we execute the normal program */
   } else {
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

      /*--------VERIFYING HTTP DATA-----------*/

      szResponseHeader[MAX_HTTP_HEADER] = '\0';

      /* Find start of ContentLength */
      char szReader[MAX_HTTP_HEADER] = {0};
      char c;
      int k;

      /* This loop finds the range from the first newline it detects and the following one. */
      int iHeaderLength = 1;
      int iarrLine[2], iFoundSizeIndexes = -1;
      memset(iarrLine, -1, sizeof(int) * 2);

      /* I devised this method to "safely" read any http header and find content-length */
      int i;
      for(i = 1; i < MAX_HTTP_HEADER; i++){
         /* When found \r\n\r\n, we have reached the end of the header */
         if(
            i > 4 &&
            szResponseHeader[i - 3] == '\r' && szResponseHeader[i - 2] == '\n' &&
            szResponseHeader[i - 1] == '\r' && szResponseHeader[i] == '\n'
         ) {
            iHeaderLength = i + 1;
            printf("HEADER SIZE: %d\n", iHeaderLength);
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
                  //printf("Content-Length found!\n");
                  /* If found, we store the location of where the number would start " ". 
                   * Where the number will end is already stored in the second index*/
                  iarrLine[0] += 16;
                  iFoundSizeIndexes = 1;
               }
            }
         }
      }

      int iDigits = iarrLine[1] - iarrLine[0], n;
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

         /* Check next digit */
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
      printf("CONTENT SIZE: %d\n", iContentLength);

      /* Allocating struct to hold response. Divided into the header (string array) and encrypted message (long array) */
      char *szHeader = NULL;

      szHeader = (char *) malloc(iHeaderLength);
      if(szHeader == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      aun_by8Encrypted = (union UN_BY8 *) malloc(iContentLength);
      if(aun_by8Encrypted == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      /* Receive header first */ 
      iReceived = recv(sockClient, szHeader, iHeaderLength, MSG_WAITALL);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      }

      /* Then receive the encrypted message in 64 bit array first */
      iReceived = recv(sockClient, aun_by8Encrypted, iContentLength, MSG_WAITALL);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 

      printf("\nHEADER=");
      for(i = 0; i < iHeaderLength; i++){
         if(szHeader[i] == '\r') printf("\\r");
         else if(szHeader[i] == '\n') printf("\\n");
         else printf("%c", szHeader[i]);
      }
      puts("HEADER END\n");

      /* Opening / creating file handle */
      FILE *fpEncryptedFile = NULL;
      fpEncryptedFile = fopen("encrypted.enc", "wb");
      if(fpEncryptedFile == NULL){
         free(szHeader);
         szHeader = NULL;
         free(aun_by8Encrypted);
         aun_by8Encrypted = NULL;
         close(sockClient); sockClient = -1;
         return -1;
      }

      /* Writing contents of body to encrypted file in 8 byte segments (each encrypted character)*/
      fwrite(aun_by8Encrypted, sizeof(BY8), iContentLength / 8, fpEncryptedFile);

      /* Closing file and cleaning up */
      fclose(fpEncryptedFile);
      fpEncryptedFile = NULL;

      free(szHeader);
      szHeader = NULL;
   }



   close(sockClient);
   sockClient = -1;


   /*-------------PART 2: DECRYPTING THE FILE---------------------*/

   
   /* Original file was encrypted with 64 bit padding, meaning the file is actually 648 / 8 = 81 */
   int iSize = iContentLength / 8; 

   /* Change this to toggle between checking one, or all the bytes */
   int iCheckAll = 0;

   unsigned char cKeyChar = 0;

   printf("\nENC (%d bytes, %d sz) AS HEX, DECRYPT START WRITE=\n", iContentLength, iSize);
   int i;
   for(i = 0; i < iSize; i++){
      printf("%016lX ", aun_by8Encrypted[i].by8Base);
   }
   printf("\nENC AFTER WRITE END=\n\n");

   char *szDeciphered = NULL;

   /* Declaring buffer */
   union UN_BY8 *aun_by8Buffer = NULL;

   /* Checks for every 8 bit number */
   int iFailed = 1;
   for(i = 0; i <= 255; i++){

      /* Allocating for decipher container */
      char *szDeciphered = (char *) malloc(iSize + 1);
      if(szDeciphered == NULL){
         free(aun_by8Encrypted);
         aun_by8Encrypted = NULL;
         return -1;
      }
      memset(szDeciphered, 0, iSize);

      char cCheckedCharKey = (unsigned char) i;

      /* since every byte is the same, this is fine just for padding for one key modifier later */
      union UN_BY16 by16Key; 
      memset(by16Key.by4, cCheckedCharKey, sizeof(unsigned int) * 4);

      //printf("\nThe key value: 0x%08lx\n", by16Key.by4[0]);

      /* Stored as a union array for more flexibility */
      aun_by8Buffer = (union UN_BY8 *) malloc(iContentLength);
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

         /* Also attemted to convert to little endian, 
          * but this just led to another scrambled result.
          * Did not work before nor after decryption. */
         //if(l == 0){ printf("1. %08X %08X\n",un_by8Deciphered.by4[0],un_by8Deciphered.by4[1]); }
         /*
         if(t == 1){
            aun_by8Encrypted[l].by4[0] = ntohl(aun_by8Encrypted[l].by4[0]);
            aun_by8Encrypted[l].by4[1] = ntohl(aun_by8Encrypted[l].by4[1]);
         }
         */

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
         //decipher(aun_by8Encrypted[l].by4, un_by8Deciphered.by4, by16Key.by4);

         /* Store the encrypted byte back into their original indices */
         un_by8Deciphered.by4[0] = byY;
         un_by8Deciphered.by4[1] = byZ;

         /* If flagged to check one byte, go here */
         if(iCheckAll == 1){
            aun_by8Buffer[l].by4[0] = un_by8Deciphered.by4[0]; 
            aun_by8Buffer[l].by4[1] = un_by8Deciphered.by4[1]; 

            /* Check every single byte in the long for valid bytes and print if true */
            int u;
            for(u = 0; u <= 7; u++){
               /* If we found non readable ascii we break the loop */
               if(isNonReadableAscii((BYTE) aun_by8Buffer[l].by[u]) != 0){
                  iFailed = 1;
                  break;
               }
               /* If we got here, that means the long passed the test */
               if(u == 7){
                  printf("\nEncrypted Byte, on key=%d:0x%016lx\n", i, aun_by8Encrypted[l].by8Base);
                  printf("Attempted decipher, on key=%d:0x%016lx\n\n", i, aun_by8Buffer[l].by8Base);
               }
            }
            break;

         } else {
            /* Store the decyphered byte in its correlating position in the buffer */
            aun_by8Buffer[l].by8Base = un_by8Deciphered.by8Base; 

            /* Which specific byte to store */
            int iByteToSave = 7;

            /* Checks if it contains invalid characters */
            if(isNonReadableAscii((BYTE) aun_by8Buffer[l].by8Base) != 0){
               break;
            }

            /* Stores the decrypyted byte (VERIFY WHOLE 64 BYTE FIRST, DO THIS LATER)*/
            szDeciphered[l] = (char) aun_by8Buffer[l].by[0];

            /* Increments l, check next padded byte */
            l++;
         }
      }

   
      if(iCheckAll != 1){
         /* Since we are done reading data we terminate the string */
         szDeciphered[l] = '\0';
         if(l == iSize){
            /* Verifies iterator l */
            printf("---RAN SUCCESSFUL DECIPHER(%d)\n->ON KEY=%016lx%016lx \n", l, by16Key.by8[0], by16Key.by8[1]);

            /* Prints all raw decyphered bytes to the terminal */
            printf("RESULT=%s\n\n", szDeciphered);
            int a;
            printf("DECYPHERED BYTES\n");
            for(a = 0; a < iSize; a++){
               printf("%016lX ", aun_by8Buffer[a].by8Base);
            }
            printf("\n=DECYPHERED BYTES END");
            iFailed = 0;

            /* Store decyphered file */
            FILE *fpTextFile = NULL;
            fpTextFile = fopen("decrypted.txt", "w");
            if(fpTextFile == NULL){
               free(szDeciphered);
               szDeciphered = NULL;

               free(aun_by8Buffer);
               aun_by8Buffer = NULL;
               break;
            }
            
            fprintf(fpTextFile, "%s", szDeciphered);

            fclose(fpTextFile);
            fpTextFile = NULL;

            free(szDeciphered);
            szDeciphered = NULL;

            free(aun_by8Buffer);
            aun_by8Buffer = NULL;

            break;
         } else iFailed = 1;
      }

      free(szDeciphered);
      szDeciphered = NULL;

      free(aun_by8Buffer);
      aun_by8Buffer = NULL;
   }

   if(iFailed == 1) printf("\n\nDECYPHER FAILED. EXITING ...\n");

   /* Ensures there are no leaks */
   if(szDeciphered != NULL){
      free(szDeciphered);
      szDeciphered = NULL;
   }
   if(aun_by8Buffer != NULL) {
      free(aun_by8Buffer);
      aun_by8Buffer = NULL;
   }

   free(aun_by8Encrypted);
   aun_by8Encrypted = NULL;

   return 0;
}


