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
 * Function to check whether the given character is a readable ascii character.
 * Used as a whitelist.
 * */
int isNonReadableAscii(char c){
   if((127 > c && c > 30) || c == '\n' || c == '\r' || c == '\t'){
      return 0;
   }
   return 1;
}

/* NOTE: As i probably mentioned in that source file, the code was essentially entirely based on
the supplied tea.c file and its documentation. */
void decipher(unsigned int *const v, unsigned int *const w, unsigned int *const k, unsigned int n){
   register unsigned int y=v[0], z=v[1], delta=0x9E3779B9, sum=delta * n; /*sum=0xC6EF3720**/
   register unsigned int a=k[0], b=k[1], c=k[2], d=k[3];

   int i; 
   for(i = 0; i < n; i++){
      z -= ((y << 4) + c) ^ (y+sum) ^ ((y>>5) + d);
      y -= ((z << 4) + a) ^ (z+sum) ^ ((z>>5) + b);
      sum -= delta;
   }

   w[0] = y;
   w[1] = z;
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
      char *szHeader = NULL;
      union UN_BY8 *arrby8Encrypted = NULL;

      szHeader = (unsigned char *) malloc(iHeaderLength);
      if(szHeader == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      arrby8Encrypted = (union UN_BY8 *) malloc(iContentLength);
      if(arrby8Encrypted == NULL){
         printf("Malloc failed. Exiting ...\n");
         close(sockClient); sockClient = -1;
         return -3;
      }

      /* Receive header first */
      iReceived = recv(sockClient, szHeader, iHeaderLength, 0);
      if(iReceived < 0){
         printf("Failed to receive response from server - errcode %d\n", errno);
         close(sockClient); sockClient = -1;
         return -2; 
      } 

      /* Then receive the encrypted message in 64 bit array first */
      iReceived = recv(sockClient, arrby8Encrypted, iContentLength, 0);
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

      printf("\nENC AS c=\n");
      for(i = 0; i < iContentLength / 8; i++){
         printf("0x%016llX ", arrby8Encrypted[i].by8Base);
         /* Bytes are converted to LITTLE ENDIAN */
         //reRequest.arrby8Encrypted[i].by4[0] = ntohl(reRequest.arrby8Encrypted[i].by4[0]);
         //reRequest.arrby8Encrypted[i].by4[1] = ntohl(reRequest.arrby8Encrypted[i].by4[1]);
      }
      printf("\nENC END=\n");

      /* Opening / creating file handle */
      FILE *fpEncryptedFile = NULL;
      fpEncryptedFile = fopen("./encrypted.bin", "wb");
      if(fpEncryptedFile == NULL){
         free(szHeader);
         szHeader = NULL;
         free(arrby8Encrypted);
         arrby8Encrypted = NULL;
         close(sockClient); sockClient = -1;
         return -1;
      }

      printf("\nENC (%d bytes) AS HEX, AFTER WRITE=\n", iContentLength);
      for(i = 0; i < iContentLength / 8 ; i++){
         printf("%016llX ", arrby8Encrypted[i].by8Base);
      }
      printf("\nENC AFTER WRITE END=\n");

      /* Writing contents of body to encrypted file in 8 byte segments (each encrypted character)*/
      fwrite(arrby8Encrypted, sizeof(BY8), iContentLength / 8, fpEncryptedFile);

      /* Closing file and cleaning up */
      fclose(fpEncryptedFile);
      fpEncryptedFile = NULL;

      free(szHeader);
      szHeader = NULL;
      free(arrby8Encrypted);
      arrby8Encrypted = NULL;
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
   unsigned char cKeyChar = 0;

   printf("\nENC (%d bytes, %d sz) AS HEX, AFTER WRITE=\n", iFileContent, iSize);
   int i;
   for(i = 0; i < iSize; i++){
      printf("%016llX ", aun_by8Encrypted[i].by8Base);
   }
   printf("\nENC AFTER WRITE END=\n");

   char *szDeciphered = NULL;

   /* Runs the decryption for four extra iterations.
    * 0. No endian conversion
    * 1. ntohl pre decipher (network to host)
    * 2. ntohl post decipher (network to host)
    * */
   int t;
   for(t = 0; t <= 2; t++){

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
         memset(by16Key.by, (BYTE) cCheckedCharKey, 16);

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

            /* Structure to hold the decrypted bye */
            union UN_BY8 un_by8Deciphered;
            un_by8Deciphered.by4[0] = 0;
            un_by8Deciphered.by4[1] = 0;
            /* Also attemted to convert to little endian, but result scrambled identically.
             * Converting before decryption also led to no valid results. */
            //if(l == 0){ printf("1. %08X %08X\n",un_by8Deciphered.by4[0],un_by8Deciphered.by4[1]); }
            if(t == 1){
               aun_by8Encrypted[l].by4[0] = ntohl(aun_by8Encrypted[l].by4[0]);
               aun_by8Encrypted[l].by4[1] = ntohl(aun_by8Encrypted[l].by4[1]);
            }
            /* [SUPPOSED TO] decipher the given long by split integer (4,4). The key is always the same
             * integer for this task since every byte in the key is the same. Decrypted long is stored in
             * the Deciphered union.*/
            decipher(aun_by8Encrypted[l].by4, un_by8Deciphered.by4, by16Key.by4, 32);

            /* Also attemted to convert to little endian, but result scrambled identically.
             * Converting before decryption also led to no valid results. */
            if(t == 2){
               aun_by8Buffer[l].by4[0] = ntohl(un_by8Deciphered.by4[0]);
               aun_by8Buffer[l].by4[1] = ntohl(un_by8Deciphered.by4[1]);
            }
            //if(l == 0){ printf("2. %08X %08X\n",un_by8Deciphered.by4[0],un_by8Deciphered.by4[1]); }
            /* Checks the first char, given padding goes 0xVA 0x07 0x07 0x07 0x07 0x07 0x07 0x07 
             * This could change as a result of endianness */

            /* In iteration 0 we assign to the buffer normally */
            if(t == 0){
               aun_by8Buffer[l].by4[0] = un_by8Deciphered.by4[0]; 
               aun_by8Buffer[l].by4[1] = un_by8Deciphered.by4[1]; 
            }

            szDeciphered[l] = (char) aun_by8Buffer[l].by[0];
            l++;
         }

         //printf("\nDEC (%d bytes, KEY=%d ) AS HEX, AFTER WRITE=\n", iFileContent, i);
         //printf("%016llX ", aun_by8Buffer[0].by8Base);
         //printf("\nDEC AFTER WRITE END=\n");

         szDeciphered[l] = '\0';

         int iChars;
         for(iChars = 0; iChars <= l; iChars++){
            if(isNonReadableAscii(szDeciphered[iChars])){
               iFailed = 1;
            }
         }

         if(iFailed != 1){
            printf("\n\n---RAN DECRYPT -> TEA ITERATIONS=%d, ENDIAN=%d, KEY=%02x\n", iIterations, t, cCheckedCharKey);
            printf("\nSolution? %s\n\n", szDeciphered);
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


