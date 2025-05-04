/*
 * TITLE: Threading task 1
 * DESCRIPTION
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

typedef struct {
   /* Synchronization controllers */
   pthread_mutex_t muMutex;
   sem_t semReaderDone;
   sem_t semCounterDone;

   /* Shared data */
   int iarrByteCount[BYTE_RANGE];
   BYTE byarrBuffer[BUFFER_SIZE];
   int iBytesInBuffer; /* TODO: Rename this */

   /* Filename for reader thread */
   char *szFileName;
} THREAD_ARGS;

/* Renamed from A to Reader thread (since its main job reading from file to the buffer) */
void* ReaderThread(void* vpArgs) {
   /* Declare variables */
   FILE *fp = NULL;
   int iBytesRead = 0;
   int iFileSize = 0;

   /* Typecast pointer to arg struct (THEAD_ARGS) TODO: Change variable name, change type to STRUCT* and store a reference for each field locally for more clean code */
   THREAD_ARGS targArgs = *(THREAD_ARGS *) vpArgs; 


   /* Opens file for reading */
   fp = fopen(targArgs.szFileName, "rb");
   if (fp == NULL) {
      perror("Failed to open file");
      exit(EXIT_FAILURE);
   }

   /* Retrieve the size of file */
   if(fseek(fp, 0, SEEK_END) == 0){
      iFileSize = ftell(fp);
      rewind(fp);
   }

   printf("READER: Thread started!\n");
   /* Runs thread execution until break condition */
   while (1) {
      /* Since data from previos loop has been read, we empty the memory buffer */
      memset(targArgs.byarrBuffer, 0, BUFFER_SIZE);

      /* Reads data into */
      iBytesRead += fread((targArgs.byarrBuffer + targArgs.iBytesInBuffer), 1, BUFFER_SIZE - targArgs.iBytesInBuffer, fp);

      /* Increments bytes shared bytes read counter by the bytes read during this loop */
      targArgs.iBytesInBuffer++;

      /* Break condition, should break when bytes read is equal to file size!! */
      if(iBytesRead > iFileSize) {
         /* When the whole file has been read, signal counter to count remaining data */
         printf("READER: No more bytes to read! Signaling counter to read remaining bytes.\n");
         sem_post(&targArgs.semReaderDone);
         pthread_mutex_unlock(&targArgs.muMutex);

         /* And exit the loop */
         break;
      }

      /* If bytes read have hit the max buffer, signal the counter to start */
      /* Signals counter thread to start */
      if(targArgs.iBytesInBuffer == BUFFER_SIZE){

         /* Unlocks the mutex and signals other thread to continue */
         /* NOTE: Moved the mutex unlock to before we signal other thread to start, to avoid the thread attempting to
          * modify data that is locked by the mutex */
         pthread_mutex_unlock(&targArgs.muMutex);

         /* Increment the semaphore */
         printf("READER: Buffer is full! Signaling counter to start.\n");
         sem_post(&targArgs.semReaderDone);

         /* Wait for available semaphore */
         printf("READER: Waiting for signal from counter...\n");
         sem_wait(&targArgs.semCounterDone);

         /* Locks the mutex to prevent data from being changed while running */
         pthread_mutex_lock(&targArgs.muMutex);
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
   THREAD_ARGS targArgs = *(THREAD_ARGS *) vpArgs; 
   int i;

   /* Initializing counter array before counter loop starts */ 
   memset(targArgs.iarrByteCount, 0, sizeof(int) * BYTE_RANGE);

   printf("COUNTER: Thread started!\n");
   /* Runs counter infinitely, or until break condition */
   while (1) {

      /* waits for signal from reader */
      printf("COUNTER: Waiting for signal from reader ...\n");
      sem_wait(&targArgs.semReaderDone);

      /* Locks mutex while counter is running to prevent unexpected changes */
      pthread_mutex_lock(&targArgs.muMutex);
      printf("COUNTER: Signal received! Starting counter...\n");
      /* "Signals" the reader to start again */

      /* If bytes in buffer is still zero after reader has handled it,
       * Break loop, theres no more data to count */
      if(targArgs.iBytesInBuffer == 0){
         printf("COUNTER: Buffer is empty! exiting counter ...\n");
         break;
      }

      /* Counts the number of a byte's occurence in the buffer */
      for(i = 0; i < targArgs.iBytesInBuffer; i++){

         /* Indexes by the BYTE (value between 0 and MAX_RANGE) and increments that value */
         (targArgs.iarrByteCount[targArgs.byarrBuffer[i]])++;
      }

      printf("Bytes added to list!\n");
      /* Resets the number of bytes in buffer */
      targArgs.iBytesInBuffer = 0;

      printf("COUNTER: Buffer is empty! Signaling reader to start.\n");
      pthread_mutex_unlock(&targArgs.muMutex);
      sem_post(&targArgs.semCounterDone);
   }

   /* Prints counter results to the terminal */
   for(i = 0; i < BYTE_RANGE; i++)
      printf("%d: %d\n", i, targArgs.iarrByteCount[i]);

   /* Closes thread */
   pthread_exit(NULL);
}

int main(int iArgC, char **arrpszArgV) {
   /* Declaring variables */
   pthread_t thrReader, thrCounter;
   THREAD_ARGS *targArgs = NULL;

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
   targArgs = (THREAD_ARGS *) malloc(sizeof(THREAD_ARGS));
   if(targArgs == NULL){
      perror("Malloc to thread data failed.");
      exit(1);
   }


   /* Passes the given filename into thread data struct */
   targArgs->szFileName = arrpszArgV[1];

   /* Initialize data */
   targArgs->iBytesInBuffer = 0;

   /* Initialize mutex (with default attributes) */
   pthread_mutex_init(&targArgs->muMutex, NULL);

   /* Since the semaphores only are going to communicate with each other,
    * and there only is two threads, we set their (counter) to 0 */
   sem_init(&targArgs->semReaderDone, 0, 0);

   /* NOTE: This one is initialized to zero so that reader can start :) */
   sem_init(&targArgs->semCounterDone, 0, 0);

   /* Start threads */
   printf("Starting reader thread.\n");
   if(pthread_create(&thrReader, NULL, ReaderThread, (void *) targArgs) != 0) {
      perror("Could not create thread A");
      free(targArgs);
      exit(1);
   }

   printf("Starting counter thread.\n");
   if(pthread_create(&thrCounter, NULL, CounterThread, (void *) targArgs) != 0) {
      perror("Could not create thread B");
      free(targArgs);
      exit(1);
   }

   /* Wait for reader thread to finish first */
   if(pthread_join(thrReader, NULL) != 0) {
      perror("Could not join thread A");
      free(targArgs);
      exit(1);
   }
   /* Then for counter thread to finish */
   if(pthread_join(thrCounter, NULL) != 0) {
      perror("Could not join thread B");
      free(targArgs);
      exit(1);
   }

   /* Destroy synchronization elements */
   pthread_mutex_destroy(&targArgs->muMutex);
   sem_destroy(&targArgs->semReaderDone); 
   sem_destroy(&targArgs->semCounterDone); 

   /* Destroy thread data struct */
   free(targArgs);

   return 0;
}










