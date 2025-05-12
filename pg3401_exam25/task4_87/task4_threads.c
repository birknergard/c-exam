/*
 * TITLE: Task 4: Threading
 * DESCRIPTION
 * My solution to task 4 part I and II 
 * Since it says in the exam description that I am not to create any files,
 * ive put everything I would have put in a header file in this file.
 *
 * I was not sure if the threads were meant to read/count from the buffer simultaneously.
 * I could nbot glean from neither the task description nor the original file whether this was
 * the intent, so i created it so that it essentially works sequentially. The counter thread and
 * the reader thread will never read the memory buffer simultaneously. While one of them works, the other
 * waits, until the signal is given, where they will switch.
 *
 * I also was not sure whether we were supposed to transform the decryption code in any way. I attempted to do
 * so and tested the result but I would love to know in the feedback.
 * */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h> /* Semaphores are not in pthread.h so we add it here */

#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define NUM_THREADS 2
#define BYTE_RANGE 256
#define MAX_FILENAME 1028

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

/* Defining BYTE types for clarity */
#define BYTE unsigned char

union _KEY {
   long split[2];
   BYTE bytes[16];
} KEY;

union ENCBYTE{
   BYTE by[8];
   unsigned int by4[2];
   long by8;
}; 


typedef struct _THREAD_ARGS {
   /* Synchronization controllers */
   pthread_mutex_t muLock;
   sem_t semReaderDone;
   sem_t semCounterDone;

   /* Shared data */
   BYTE byarrBuffer[BUFFER_SIZE];
   int iBytesInBuffer; 

   /* For initial handshake, only to be edited by their respective threads. 1 if TRUE, 0 if FALSE */ 
   int iReaderSetupFailed;
   int iCounterSetupFailed;

   /* Filename for reader thread */
   char *szFileName;

} THREAD_ARGS;

/*
 * Checks a string if it has the .txt file suffix 
 * returns 1 if true, 0 if false 
 * */
int isTxtFile(char *szFileName);

/* Renamed from A to Reader thread (since its main job reading from file to the buffer) */
void* ReaderThread(void* vpArgs) {
   /* Declare variables */
   int iTotalBytesRead = 0;
   int iFileSize = 0;

   /* Declare file pointer */
   FILE *fpSource = NULL;

   /* Typecast pointer to arg struct (THEAD_ARGS) 
    * and store a reference for each field locally for more clean code */
   THREAD_ARGS *tData = (THREAD_ARGS *) vpArgs; 

   /* Attempts to open source file (lock briefly lock mutex because we are accessing filename) */
   printf("READER: Attempting to open source file ...\n");
   pthread_mutex_lock(&tData->muLock);
   fpSource = fopen(tData->szFileName, "r");
   if (fpSource == NULL) {
      perror("Failed to open file");

      /* Store outcome of setup in shared memory */
      tData->iReaderSetupFailed = 1;
   } else {
      /* Retrieve the size of file */
      if(fseek(fpSource, 0, SEEK_END) == 0){
         iFileSize = ftell(fpSource);
         rewind(fpSource);
      }
      printf("READER: File opened successfully (%d bytes), signaling counter ...\n", iFileSize);
   }

   /* Signal counter to read setup outcome */
   sem_post(&tData->semReaderDone);

   if(tData->iReaderSetupFailed != 1){
      /* Waits for counter to signal that it succeeded in opening */
      pthread_mutex_unlock(&tData->muLock);
      sem_wait(&tData->semCounterDone);
      printf("READER: Signal received from counter, checking if succesful ...\n");

      pthread_mutex_lock(&tData->muLock);

      /* If counter didn't fail, start reader */
      if(tData->iCounterSetupFailed != 1) {
         printf("READER: Counter was successful! Starting main reader process.\n");

         /* Runs thread execution until break condition */
         while (1) {

            /* Reads one byte at a time from the file into the byte array TODO: Make it read more bytes at a time,
             * this is ineffiecient */
            tData->iBytesInBuffer += fread((tData->byarrBuffer + tData->iBytesInBuffer), 1, BUFFER_SIZE, fpSource);

            /* Increments bytes shared bytes read counter by the bytes read during this loop */
            iTotalBytesRead += tData->iBytesInBuffer;

            /* Break condition, should break when bytes read is equal to file size!! */
            if(iTotalBytesRead == iFileSize) {

               /* When the whole file has been read, signal counter to count remaining data */
               printf("READER: No more bytes to read! Signaling counter to read remaining bytes.\n");

               /* Signal counter to continue */
               sem_post(&tData->semReaderDone);
               pthread_mutex_unlock(&tData->muLock);

               /* And exit the loop */
               break;
            }

            /* If bytes read have hit the max buffer, signal the counter to start */
            if(tData->iBytesInBuffer == BUFFER_SIZE){

               printf("READER: Buffer is full! Signaling counter to start.\n");

               /* Increment the semaphore */
               sem_post(&tData->semReaderDone);
               pthread_mutex_unlock(&tData->muLock);

               /* Wait for available semaphore */
               printf("READER: Waiting for signal from counter...\n");

               /* Signals counter thread to start */
               sem_wait(&tData->semCounterDone);

               /* Locks the mutex to prevent data from being changed while running */
               pthread_mutex_lock(&tData->muLock);

               /* Since data from previos loop has been read, we empty the memory buffer */
               memset(tData->byarrBuffer, 0, BUFFER_SIZE);
               printf("READER: Signal received! Continuing reader ...\n");
            }
         } /*-> READER ENDLOOP*/
      } else {
         printf("READER: Counter setup failed! Exiting ...\n");
      } /*-> COUNTER SETUP SUCCESFUL ENDIF*/
      pthread_mutex_unlock(&tData->muLock);

      /* Close file handle and set file pointer to NULL */
      fclose(fpSource);
      fpSource = NULL;

   } /*-> SOURCE FILE OPENED SUCCESSFULLY ENDIF*/

   /* Exits thread */
   pthread_exit(NULL);
}

/* Renamed from B to Counter (since its main job is counting each byte in the buffer) */
void* CounterThread(void* vpArgs) {
   /* Declaring variables */
   int i;
   int iReaderComplete = 0;
   int iarrByteCount[BYTE_RANGE];
   int iDJB2Hash; 
   BYTE by, byKey[16];

   /* Casting void * to argument struct * */
   THREAD_ARGS *tData = (THREAD_ARGS *) vpArgs; 

   /* Declaring file pointers */
   FILE *fpHashed = NULL;
   FILE *fpEncrypted = NULL;


   printf("COUNTER: Waiting for signal from reader ...\n");
   /* Wait for signal from reader */
   sem_wait(&tData->semReaderDone);
   pthread_mutex_lock(&tData->muLock);

   /* Only run if neither reader nor counter failed setup */
   if(tData->iReaderSetupFailed != 1){
      printf("COUNTER: Signal received! Reader setup was successful! Opening hash and encrypt files...\n");

      /* Opening/creating files, sets flag to failure if failed to prevent reader thread from running */
      fpHashed = fopen("./task4_pg2265.hash", "wb");
      if(fpHashed == NULL){
         perror("Failed to open .hash file.");

         /* Set failure flag to true */
         tData->iCounterSetupFailed = 1;
      }
      
      fpEncrypted = fopen("./task4_pg2265.bin", "wb");
      if(fpEncrypted == NULL){
         perror("Failed to open .enc file.");

         /* Set failure flag to true */
         tData->iCounterSetupFailed = 1;
      }

      printf("COUNTER: Attempted open of .hash and .enc files! Signaling reader thread with result ...\n");

      pthread_mutex_unlock(&tData->muLock);
      /* Signal reader to start operations */
      sem_post(&tData->semCounterDone);

      if(tData->iCounterSetupFailed != 1){

         /* Initializing counter array before counter loop starts */ 
         memset(iarrByteCount, 0, sizeof(int) * BYTE_RANGE);

         /* Runs counter infinitely, or until break condition */
         while(1){

            /* Waits for available semaphore from reader */
            sem_wait(&tData->semReaderDone);

            /* Locks mutex while counter is running to prevent unexpected changes */
            pthread_mutex_lock(&tData->muLock);

            /* If bytes in buffer is less than max, that means the reader didnt have more
             * data to read. therefore we can break the loop when this happens. */
            if(tData->iBytesInBuffer < BUFFER_SIZE){
               printf("COUNTER: Reader is done reading the file! Completing count ...\n");
               iReaderComplete = 1;
            }

            /* DJB2 HASH */
            for(i = 0; i < tData->iBytesInBuffer; i++){
               /* Stores byte to hash in BYTE variable (by) */
               by = tData->byarrBuffer[i];

               /* Hash the byte value */
               iDJB2Hash = 5381;
               by = ((iDJB2Hash << 5) + iDJB2Hash) + by;

               /* Write hashed byte value to file */
               fwrite(&by, 1, 1, fpHashed);
            }

            /* TEA ENCRYPT */
            for(i = 0; i < tData->iBytesInBuffer; i++){
               by = tData->byarrBuffer[i];

               /* Create key and padded byte */
               memset(byKey, '1', 16);

               /* Pad byte with union */
               union ENCBYTE byEncrypted;
               memset(&byEncrypted.by, 0x07, 8);
               byEncrypted.by[0] = by;

               /* Running algorithm (Made by David Wheeler and Roger Needham, provided by EWA) 
                * I wasn't sure if I should include register keyword on the variable declarations, since
                * we havent learned it during the course. */

               unsigned int uiSum = 0, uiDelta = 0x9E3779B9;
               unsigned int uiKeyOne = byKey[3], uiKeyTwo = byKey[7], uiKeyThree = byKey[11], uiKeyFour = byKey[15];
               int n;

               /* Encrypts padded byte */
               for(n = 0; n < 32; n++){
                  uiSum += uiDelta;
                  byEncrypted.by4[0] += ((byEncrypted.by4[1] << 4) + uiKeyOne) ^ (byEncrypted.by4[1] + uiSum) ^ (( byEncrypted.by4[1] >> 5) + uiKeyTwo);
                  byEncrypted.by4[1] += ((byEncrypted.by4[0] << 4) + uiKeyThree) ^ (byEncrypted.by4[0] + uiSum) ^ (( byEncrypted.by4[0] >> 5) + uiKeyFour);
               }

               byEncrypted.by8 = htonll(byEncrypted.by8);
               /* Write encrypted byte to file */
               fwrite(&byEncrypted.by8, sizeof(BYTE) * 8, 1, fpEncrypted);
            }

            /* Resets the number of bytes in buffer */
            tData->iBytesInBuffer = 0;

            /* If bytes were less than max when signal was received, we exit the program here. */
            pthread_mutex_unlock(&tData->muLock);
            if(iReaderComplete == 1){
               break;
            } else {
               sem_post(&tData->semCounterDone);

            }
         } /*-> COUNTER ENDLOOP */
      } /*-> COUNTER SETUP WAS SUCCESSFUL ENDIF*/ 

      /* Closing .hash (if its open) */
      if(fpHashed != NULL){
         fclose(fpHashed);
         fpHashed = NULL;
      }

      /* Same for .enc */
      if(fpEncrypted != NULL){
         fclose(fpEncrypted);
         fpEncrypted = NULL;
      }
   } /*-> READER WAS SUCCESSFUL ENDIF */


   /* Closes thread */
   pthread_exit(NULL);
}

int main(int iArgC, char **arrpszArgV) {
   /* Declaring variables */
   pthread_t thrReader, thrCounter;
   THREAD_ARGS *tData = NULL; /* (t)hread Data */

   printf("Preparing program.\n");
   /* Next two statements are there to verify argv. Whether it exists at all ... */
   if(arrpszArgV[1] == NULL){
      perror("No filename was provided to the program. Requires a file name.");
      exit(EXIT_FAILURE);
   }

   /* Or if its not within required boundaries for program ... */
   if(strlen(arrpszArgV[1]) > MAX_FILENAME || strlen(arrpszArgV[1]) == 1){
      perror("Given filename is not within character limit. (Greater than 1, less than 1028)");
      exit(EXIT_FAILURE);
   }

   if(isTxtFile(arrpszArgV[1]) == 0){
      perror("Given file is not a .txt file.");
      exit(EXIT_FAILURE);
   }

   /* Since source file is passed from user, ensure user can only open .txt files
      If user attempts to open .enc or .hash file it could cause major issues, so we just restrict
      user to .txt files */

   /* Allocating thread data */
   tData = (THREAD_ARGS *) malloc(sizeof(THREAD_ARGS));
   if(tData == NULL){
      perror("Malloc to thread data failed.");
      exit(EXIT_FAILURE);
   }

   /* Passes the given filename into thread data struct */
   tData->szFileName = arrpszArgV[1];

   /* Initialize data */
   tData->iBytesInBuffer = 0;
   
   /* Set initial boolean flags */
   tData->iReaderSetupFailed = -1;
   tData->iCounterSetupFailed = -1;

   /* Initialize mutex (with default attributes) */
   pthread_mutex_init(&tData->muLock, NULL);

   /* Since the semaphores only are going to communicate with each other,
    * and there only is two threads, we set their (counter) to 0 */
   sem_init(&tData->semReaderDone, 0, 0);
   sem_init(&tData->semCounterDone, 0, 0);

   /* Start threads */
   printf("Starting reader thread.\n");
   if(pthread_create(&thrReader, NULL, ReaderThread, (void *) tData) != 0) {
      perror("Could not create thread A");
      free(tData);
      exit(EXIT_FAILURE);
   }

   printf("Starting counter thread.\n");
   if(pthread_create(&thrCounter, NULL, CounterThread, (void *) tData) != 0) {
      perror("Could not create thread B");
      free(tData);
      exit(EXIT_FAILURE);
   }

   /* Wait for reader thread to finish first */
   if(pthread_join(thrReader, NULL) != 0) {
      perror("Could not join thread A");
      free(tData);
      exit(EXIT_FAILURE);
   }
   /* Then for counter thread to finish */
   if(pthread_join(thrCounter, NULL) != 0) {
      perror("Could not join thread B");
      free(tData);
      exit(EXIT_FAILURE);
   }

   /* Destroy synchronization elements */
   pthread_mutex_destroy(&tData->muLock);
   sem_destroy(&tData->semReaderDone); 
   sem_destroy(&tData->semCounterDone); 

   /* Destroy thread data struct */
   free(tData);

   return 0;
}

/*
 * Checks if filename given is of the .txt type
 * */
int isTxtFile(char *szFileName){
   int iLength = strlen(szFileName);
   int i, j;
   /* One extra byte is added to account for '\0' */
   char szSubstring[5];

   /* Initialize string */
   memset(szSubstring, 0, 5);

   /* Get last four letters of string */
   j = 0;
   for(i = iLength - 4; i < iLength; i++){
      szSubstring[j] = szFileName[i]; 
      j++;
   }
   szSubstring[4] = '\0';
   
   /* Check if substring == ".txt" */
   if(strncmp(szSubstring, ".txt", 4) == 0){
      return 1;
   } else return 0;
}

