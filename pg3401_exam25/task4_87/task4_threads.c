/*
 * TITLE: Threading task 1
 * DESCRIPTION
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

   /* Typecast pointer to arg struct (THEAD_ARGS) TODO: Change variable name, change type to STRUCT* and store a reference for each field locally for more clean code */
   THREAD_ARGS targArgs = *(THREAD_ARGS *) vpArgs; 


   /* Opens file for reading */
   fp = fopen(targArgs.szFileName, "rb");
   if (!fp) {
      perror("Failed to open file");
      exit(EXIT_FAILURE);
   }

   printf("READER: Thread started!\n");
   /* Runs thread execution until break condition */
   while (1) {
      /* Wait for available semaphore */
      printf("READER: Waiting for signal from counter ...\n");
      sem_wait(&targArgs.semCounterDone);

      /* Locks the mutex to prevent data from being changed while running */
      pthread_mutex_lock(&targArgs.muMutex);
      printf("READER: Starting reader...\n");

      /* Since data from previos loop has been read, we empty the memory buffer */
      memset(targData.byarrBuffer, 0, BUFFER_SIZE);

      /* Reads bytes from the file. By incrementing the bytearr by the bytes read it ignores 
       * the parts of the buffer that has been read already. TODO: Add more context to comment after testing */
      int iBytesRead = fread(targArgs.byarrBuffer + targArgs.iBytesInBuffer, 1, BUFFER_SIZE - targArgs.iBytesInBuffer, fp);

      /* Increments bytes shared bytes read counter by the bytes read during this loop */
      targArgs.iBytesInBuffer += iBytesRead;

      /* Break condition */
      if(iBytesRead > (BUFFER_SIZE - targArgs.iBytesInBuffer)) {
         /* Unlocks the mutex when thread is done. */
         pthread_mutex_unlock(&targArgs.muMutex);
         break;
      }

      /* If bytes read have hit the max buffer, signal the counter to start */
      /* Signals counter thread to start */
      if(targArgs.iBytesInBuffer == BUFFER_SIZE){
         printf("READER: Buffer is full! Signaling counter to start.\n");

         /* Unlocks the mutex and signals other thread to continue */
         /* NOTE: Moved the mutex unlock to before we signal other thread to start, to avoid the thread attempting to
          * modify data that is locked by the mutex */
         pthread_mutex_unlock(&targArgs.muMutex);

         /* Increment the semaphore */
         sem_post(&targArgs.semReaderDone);
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
   memset(targData.iarrByteCount, 0, sizeof(int) * BYTE_RANGE);

   printf("COUNTER: Thread started!\n");
   /* Runs counter infinitely, or until break condition */
   while (1) {
      /* If no bytes have been read (iBytesInBuffer) wait for signal from reader */
      printf("COUNTER: Waiting for signal from reader ...\n");
      sem_wait(&targArgs.semReaderDone);

      /* Locks mutex while counter is running to prevent unexpected changes */
      pthread_mutex_lock(&targArgs.muMutex);
      printf("COUNTER: Counting from buffer...\n");

      /* If bytes in buffer is still zero after reader has handled it,
       * Break loop, theres no more data to count */
      printf("COUNTER: Buffer is empty! exiting counter ...\n");
      if(targArgs.iBytesInBuffer == 0)
         break;

      /* Counts the number of a bytes occurences in the buffer
       * for every time it occured (iBytesInBuffer) and stores it in the int array
       * TODO: Verify/clarify */
      for(i = 0; i < targArgs.iBytesInBuffer; i++)
         targArgs.iarrByteCount[targArgs.byarrBuffer[i]]++;

      /* Resets the number of bytes in buffer */
      targArgs.iBytesInBuffer = 0;

      /* NOTE: Moved unlocking of mutex to before signal, so that the other thread 
       * doesn't run into blocking problems */

      /* "Signals" the reader to start again */
      if(targArgs.iBytesInBuffer == 0){
         pthread_mutex_unlock(&targArgs.muMutex);
         printf("COUNTER: Buffer is empty! Signaling reader to start.\n");
         sem_post(&targArgs.semCounterDone);
      }
   }

   /*Prints counter results to the terminal */
   for(i = 0; i < BYTE_RANGE; i++)
      printf("%d: %d\n", i, targArgs.iarrByteCount[i]);

   /* Closes thread */
   pthread_exit(NULL);
}

int main(int iArgC, char **arrpszArgV) {
   /* Declaring variables */
   pthread_t thrReader, thrCounter;
   THREAD_ARGS *targsData = NULL;

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
   targsData = (THREAD_ARGS *) malloc(sizeof(THREAD_ARGS));
   if(targsData == NULL){
      perror("Malloc to thread data failed.");
      exit(1);
   }


   /* Passes the given filename into thread data struct */
   targsData->szFileName = arrpszArgV[1];

   /* Initialize data */
   targsData->iBytesInBuffer = 0;

   /* Initialize mutex (with default attributes) */
   pthread_mutex_init(&targsData->muMutex, NULL);

   /* Since the semaphores only are going to communicate with each other,
    * and there only is two threads, we set their (counter) to 0 */
   sem_init(&targsData->semReaderDone, 0, 0);

   /* NOTE: This one is initialized to zero so that reader can start :) */
   sem_init(&targsData->semCounterDone, 0, 1);

   /* Start threads */
   printf("Starting reader thread.\n");
   if(pthread_create(&thrReader, NULL, ReaderThread, (void *) targsData) != 0) {
      perror("Could not create thread A");
      free(targsData);
      exit(1);
   }

   printf("Starting counter thread.\n");
   if(pthread_create(&thrCounter, NULL, CounterThread, (void *) targsData) != 0) {
      perror("Could not create thread B");
      free(targsData);
      exit(1);
   }

   /* Wait for reader thread to finish first */
   if(pthread_join(thrReader, NULL) != 0) {
      perror("Could not join thread A");
      free(targsData);
      exit(1);
   }
   /* Then for counter thread to finish */
   if(pthread_join(thrCounter, NULL) != 0) {
      perror("Could not join thread B");
      free(targsData);
      exit(1);
   }

   /* Destroy synchronization elements */
   pthread_mutex_destroy(&targsData->muMutex);
   sem_destroy(&targsData->semReaderDone); 
   sem_destroy(&targsData->semCounterDone); 

   /* Destroy thread data struct */
   free(targsData);

   return 0;
}










