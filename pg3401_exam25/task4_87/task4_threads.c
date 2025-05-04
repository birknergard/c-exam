/*
 * TITLE: Task 4: Threading
 * DESCRIPTION
 * My solution to task 4 part I and II 
 * Since it says in the exam description that I am not to create any files,
 * ive put everything I would have put in a header file in this file.
 * */

/* Part 1 Output
 *
 * */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h> /* Semaphores are not in pthread.h so we add it here */

#define BUFFER_SIZE 4096
#define NUM_THREADS 2
#define BYTE_RANGE 256
#define MAX_FILENAME 1028

/* Redefining to BYTE type for clarity */
#define BYTE unsigned char

typedef struct _THREAD_ARGS {
   /* Synchronization controllers */
   pthread_mutex_t muMutex;
   sem_t semReaderDone;
   sem_t semCounterDone;

   /* Shared data */
   int iarrByteCount[BYTE_RANGE];
   BYTE byarrBuffer[BUFFER_SIZE];
   int iBytesInBuffer; 

   /* Filename for reader thread */
   char *szFileName;
} THREAD_ARGS;

/*
 * Declare these functions, so i can define them below main
 * */
int DBJ2_Hash(FILE * fFileDescriptor, int* piHash);
void TEA_Encrypt(unsigned int *const v,unsigned int *const w, const unsigned int *const k);

/* Renamed from A to Reader thread (since its main job reading from file to the buffer) */
void* ReaderThread(void* vpArgs) {
   /* Declare variables */
   FILE *fp = NULL;
   int iBytesRead = 0;
   int iFileSize = 0;

   /* Typecast pointer to arg struct (THEAD_ARGS) 
    * and store a reference for each field locally for more clean code */
   THREAD_ARGS *tData = (THREAD_ARGS *) vpArgs; 

   /* Opens file for reading */
   fp = fopen(tData->szFileName, "r");
   if (fp == NULL) {
      perror("Failed to open file");
      exit(EXIT_FAILURE);
   }

   /* Retrieve the size of file */
   if(fseek(fp, 0, SEEK_END) == 0){
      iFileSize = ftell(fp);
      rewind(fp);
   }

   printf("READER: Thread started! File is of size %d \n", iFileSize);
   /* Runs thread execution until break condition */
   while (1) {
      /* Reads one byte at a time from the file into the byte array */
      iBytesRead += fread((tData->byarrBuffer + tData->iBytesInBuffer), 1, 1, fp);

      /* Increments bytes shared bytes read counter by the bytes read during this loop */
      tData->iBytesInBuffer++;

      /* Break condition, should break when bytes read is equal to file size!! */
      if(iBytesRead == iFileSize) {

         /* When the whole file has been read, signal counter to count remaining data */
         printf("READER: No more bytes to read! Signaling counter to read remaining bytes.\n");
         pthread_mutex_unlock(&tData->muMutex);

         /* Signal counter to continue */
         sem_post(&tData->semReaderDone);

         /* And exit the loop */
         break;
      }

      /* If bytes read have hit the max buffer, signal the counter to start */
      if(tData->iBytesInBuffer == BUFFER_SIZE){

         printf("READER: Buffer is full! Signaling counter to start.\n");

         /* Unlocks the mutex and signals other thread to continue */
         pthread_mutex_unlock(&tData->muMutex);

         /* Increment the semaphore */
         sem_post(&tData->semReaderDone);

         /* Wait for available semaphore */
         printf("READER: Waiting for signal from counter...\n");

         /* Signals counter thread to start */
         sem_wait(&tData->semCounterDone);

         /* Locks the mutex to prevent data from being changed while running */
         pthread_mutex_lock(&tData->muMutex);

         /* Since data from previos loop has been read, we empty the memory buffer */
         memset(tData->byarrBuffer, 0, BUFFER_SIZE);
         printf("READER: Signal received! Continuing reader ...\n");
      }
   }

   /* Close file handle and set file pointer to NULL */
   fclose(fp);
   fp = NULL;

   /* Exits thread */
   pthread_exit(NULL);
}

/* Renamed from B to Counter (since its main job is counting each byte in the buffer) */
void* CounterThread(void* vpArgs) {
   /* Declaring variables */
   int i, iReaderComplete = 0;

   /* Casting void * to argument struct * */
   THREAD_ARGS *tData = (THREAD_ARGS *) vpArgs; 

   /* Initializing counter array before counter loop starts */ 
   memset(tData->iarrByteCount, 0, sizeof(int) * BYTE_RANGE);

   printf("COUNTER: Thread started!\n");
   /* Runs counter infinitely, or until break condition */
   while (1) {

      /* waits for signal from reader */
      printf("COUNTER: Waiting for signal from reader ...\n");
      sem_wait(&tData->semReaderDone);

      /* Locks mutex while counter is running to prevent unexpected changes */
      pthread_mutex_lock(&tData->muMutex);
      printf("COUNTER: Signal received! Starting counter...\n");
      /* "Signals" the reader to start again */

      /* If bytes in buffer is less than max, that means the reader didnt have more
       * data to read. therefore we can break the loop when this happens. */
      if(tData->iBytesInBuffer < BUFFER_SIZE){
         printf("COUNTER: Reader is done reading the file! Completing count ...\n");
         iReaderComplete = 1;
      }

      /* Counts the number of a byte's occurence in the buffer */
      /* TODO: Task 2: Replace this with hash and encryption methods */
      for(i = 0; i < tData->iBytesInBuffer; i++){
         /* Indexes by the BYTE (value between 0 and MAX_RANGE) and increments that value */
         tData->iarrByteCount[tData->byarrBuffer[i]]++;
      }

      printf("Bytes added to list!\n");
      /* Resets the number of bytes in buffer */
      tData->iBytesInBuffer = 0;


      /* If bytes were less than max when signal was received, we exit the program here. */
      if(iReaderComplete == 1){
         pthread_mutex_unlock(&tData->muMutex);
         break;
      } else {
         printf("COUNTER: Buffer is empty! Signaling reader to start.\n");
         pthread_mutex_unlock(&tData->muMutex);
         /* Otherwise we signal back to the reader to continue */
         sem_post(&tData->semCounterDone);
      }
   }

   /* Prints counter results to the terminal */
   for(i = 0; i < BYTE_RANGE; i++){
      printf("%c: %d - ", i, tData->iarrByteCount[i]);
      if(i % 5 == 0) puts("");
   }


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
      exit(1);
   }

   /* Or if its not within required boundaries for program ... */
   if(strlen(arrpszArgV[1]) > MAX_FILENAME || strlen(arrpszArgV[1]) == 1){
      perror("Given filename is not within character limit. (Greater than 1, less than 1028)");
      exit(1);
   }

   /* Allocating thread data */
   tData = (THREAD_ARGS *) malloc(sizeof(THREAD_ARGS));
   if(tData == NULL){
      perror("Malloc to thread data failed.");
      exit(1);
   }


   /* Passes the given filename into thread data struct */
   tData->szFileName = arrpszArgV[1];

   /* Initialize data */
   tData->iBytesInBuffer = 0;

   /* Initialize mutex (with default attributes) */
   pthread_mutex_init(&tData->muMutex, NULL);

   /* Since the semaphores only are going to communicate with each other,
    * and there only is two threads, we set their (counter) to 0 */
   sem_init(&tData->semReaderDone, 0, 0);

   /* NOTE: This one is initialized to zero so that reader can start :) */
   sem_init(&tData->semCounterDone, 0, 0);

   /* Start threads */
   printf("Starting reader thread.\n");
   if(pthread_create(&thrReader, NULL, ReaderThread, (void *) tData) != 0) {
      perror("Could not create thread A");
      free(tData);
      exit(1);
   }

   printf("Starting counter thread.\n");
   if(pthread_create(&thrCounter, NULL, CounterThread, (void *) tData) != 0) {
      perror("Could not create thread B");
      free(tData);
      exit(1);
   }

   /* Wait for reader thread to finish first */
   if(pthread_join(thrReader, NULL) != 0) {
      perror("Could not join thread A");
      free(tData);
      exit(1);
   }
   /* Then for counter thread to finish */
   if(pthread_join(thrCounter, NULL) != 0) {
      perror("Could not join thread B");
      free(tData);
      exit(1);
   }

   /* Destroy synchronization elements */
   pthread_mutex_destroy(&tData->muMutex);
   sem_destroy(&tData->semReaderDone); 
   sem_destroy(&tData->semCounterDone); 

   /* Destroy thread data struct */
   free(tData);

   return 0;
}


