#include <sys/socket.h>
#include <netinet/in.h> /* For INADDR_ANY */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void){
	int iErrorDetected;
	iErrorDetected = RunServer(); 
	
	if(iErrorDetected != 0){
		berror("Server crashed - errcode: %d", iErrorDetected);			
		return 1;
	}

	return 0;
}

int RunServer(){
	/* Declaring variables */
	int iBinded, iListened, i;

	int iThreads;
	int **ppiThreadTracker;
	pthread_t *pThreads;
	THREAD_DATA **pptDataContainers;
	sem_t semThreadReady;

	int sockServerDescriptor;
	struct sockaddr_in saServerAddress;
	struct sockaddr_in saNewClientAddress;

	int iNewAddressLength;
	int iRequests, iRequestLimit;

	/* Setting number of threads */
	iThreads = 3;
	iRequestLimit = 4;

	/* Set pointers to NULL */
	ppiThreadTracker = NULL;
	pThreads = NULL;
	pptDataContainers = NULL;

	/* Open network socket (TCP/IP Protocol), as a stream */
	sockServerDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(sockServerDescriptor < 0){
		berror("Error when opening socket - errcode: %d", errno);
		close(sockServerDescriptor);
		return ERROR;
	}

	/* Sets address type */
	saServerAddress.sin_family = AF_INET; 

	/* Converts port (int) into TCP/IP byte order (from little to big endian) */
	saServerAddress.sin_port = htons(PORT); 

	/* The server listens to any available network interfaces (wifi etc.) on the computer */
	saServerAddress.sin_addr.s_addr = INADDR_ANY;  
			
	/* Bind socket to address */
	if((iBinded = bind(
		sockServerDescriptor,
		(struct sockaddr *) &saServerAddress,
		sizeof(saServerAddress)
	)) < 0){
		berror("Error with bind() - errcode %d", errno);
		close(sockServerDescriptor); sockServerDescriptor = -1;
		return ERROR;
	} 


	/* Make server listen for input */
	iListened =	listen(sockServerDescriptor, 5);
	if(iListened < 0){
		berror("Listen failed - errcode: %d", errno);
		close(sockServerDescriptor); sockServerDescriptor = -1;
		return ERROR;
	}

	pptDataContainers = (THREAD_DATA **) malloc(sizeof(THREAD_DATA *) * iThreads);
	if(pptDataContainers == NULL) {
		berror("An error occurred while allocating to thread containers.\n");
		close(sockServerDescriptor); sockServerDescriptor = -1;
		free(pThreads);
		return ERROR;
	}
	memset(pptDataContainers, 0, sizeof(THREAD_DATA *) * iThreads);

	/* Initialize thread tracker */
	ppiThreadTracker = (int **) malloc(sizeof(int *) * iThreads);
	if(ppiThreadTracker == NULL){
		berror("An error occured while allocating the thread tracker\n");
		close(sockServerDescriptor); sockServerDescriptor = -1;
		return ERROR;
	}
	
	memset(ppiThreadTracker, 0, sizeof(int) * iThreads);
	for(i = 0; i < iThreads; i++){
		ppiThreadTracker[i] = (int *) malloc(sizeof(int) * 2);
		if(ppiThreadTracker[i] == NULL){
			berror("An error occured whiile allocating the thread tracker\n");
			close(sockServerDescriptor); sockServerDescriptor = -1;
			free(pptDataContainers);

			/* Free however many pointers were allocated before error */
			for(; i >= 0; i--){
				free(ppiThreadTracker[i]);
			}
			free(ppiThreadTracker);
			return ERROR;
		}
		memset(ppiThreadTracker[i], 0, sizeof(int) * 2);
	}

	/* Allocate memory for threads */
	pThreads = (pthread_t *) malloc(sizeof(pthread_t) * iThreads);
	if(pThreads == NULL){
		close(sockServerDescriptor); sockServerDescriptor = -1;
		berror("An error occurred while allocating to thread containers.\n");
		return ERROR;
	}

	/* Initialize semaphores and mutexes */
	sem_init(&semThreadReady, 0, iThreads);

	/* Initializing request count tracker */
	iRequests = 0;

	puts("STARTING SERVER\n");
	/* Handle requests */
	while(1){
		/* Add request */
		iRequests++;
		if(iRequests > iRequestLimit){
			puts("MAX REQUESTS EXCEEDED, EXITING ... \n");
			break;
		}

		saNewClientAddress = (struct sockaddr_in) {0}; 
		iNewAddressLength = sizeof(saNewClientAddress); 

		/* Wait for empty thread */ 
		sem_wait(&semThreadReady);

		/* Thread is confirmed open above, now to find it*/
		i = -1;
		for(;;){
			/* Loops until it finds an open thread */
			i++;
			if(i == iThreads) i = 0;

			/* when open thread flag is found, start new thread */
			if(ppiThreadTracker[i][1] == 0){

				/* Closes thread if it has been active */
				if(ppiThreadTracker[i][0] == 1){
					pthread_join(pThreads[i], NULL);
				}

				//bdebug("FREEING OLD DATA FOR TD-%d if exists ...\n", i);
				if(pptDataContainers[i] != NULL){
					free(pptDataContainers[i]);
				}

				//bdebug("ALLOCATING NEW MEMORY FOR FOR TD-%d DATA ...\n", i);
				pptDataContainers[i] = (THREAD_DATA *) malloc(sizeof(THREAD_DATA));
				if(pptDataContainers[i] == NULL){
					berror("An error occurred while allocating to thread container.\n");
					close(sockServerDescriptor); sockServerDescriptor = -1;
					break;
				}
				memset(pptDataContainers[i], 0, sizeof(THREAD_DATA));

				puts("WAITING FOR CLIENT ... \n");
				pptDataContainers[i]->sockClientDescriptor = accept(
					sockServerDescriptor,
					(struct sockaddr *) &saNewClientAddress,
					(socklen_t *) &iNewAddressLength
				);
				puts("ACCEPTED CONNECTION\n");

				/* if connection was unsuccessful */ 
				if(pptDataContainers[i]->sockClientDescriptor < 0){
					berror("An error occured when accepting connection - errcode: %d\n", errno);
					break;
				}

				/* Stores a reference to our thread tracker... */
				pptDataContainers[i]->ppiThreadTracker = ppiThreadTracker; 

				/* and semaphores */
				pptDataContainers[i]->semThreadReady = &semThreadReady;

				/*bdebug("Open thread found (id:%d)! Running thread ... \n", i);*/
				pptDataContainers[i]->iThreadID = i;

				/* Mark thread has having activated */
				pptDataContainers[i]->ppiThreadTracker[i][0] = 1;

				/* Mark thread as running */
				pptDataContainers[i]->ppiThreadTracker[i][1] = 1;

				/* Create thread */
				pthread_create(&pThreads[i], NULL, HandleRequest, (void*) pptDataContainers[i]);
				break;	
			}
		}
	}

	/* Close any open threads */
	puts("WAITING FOR REMAINING THREADS ...")
	for(i = 0; i < iThreads; i++){
		if(ppiThreadTracker[i][0] == 1 || ppiThreadTracker[i][1] == 1)
			pthread_join(pThreads[i], NULL);
	}
	free(pThreads);
	pThreads = NULL;

	/* Destroy thread resources */
	sem_destroy(&semThreadReady);
	for(i = 0; i < iThreads; i++){
		if(pptDataContainers[i] != NULL){
			close(pptDataContainers[i]->sockClientDescriptor); pptDataContainers[i]->sockClientDescriptor = -1;
			free(pptDataContainers[i]);
		}
		free(ppiThreadTracker[i]);
	}
	free(ppiThreadTracker);
	free(pptDataContainers);
	pptDataContainers = NULL;

	/* Close the sockets, then assign a secure exit value */
	close(sockServerDescriptor); sockServerDescriptor = -1;
	puts("EXITING ...")

	return OK;
}

void *HandleRequest(void *vptData){
	/* Preparing for execution */
	int iReceived, iSent;
	BSCP_PACKET *bscpPacket = NULL;
	THREAD_DATA *ptData = NULL;

	ptData = (THREAD_DATA *) vptData;
	printf("TH-ID %d: THREAD STARTED! on port: %d -> TRACKER [%d, %d, %d]\n", 
		ptData->iThreadID,
		PORT,
		ptData->ppiThreadTracker[0][1],
		ptData->ppiThreadTracker[1][1],
		ptData->ppiThreadTracker[2][1]
	);

	/* Allocate memory for buffer */
	bscpPacket = (BSCP_PACKET*) malloc(sizeof(BSCP_PACKET));
	if(bscpPacket == NULL){
		ptData->ppiThreadTracker[ptData->iThreadID][1] = 0;
		sem_post(ptData->semThreadReady);
		return NULL;
	}

	memset(bscpPacket, 0, sizeof(BSCP_PACKET));

	/* Take request */
	if((iReceived = recv(ptData->sockClientDescriptor, bscpPacket, MAX_PACKET, 0)) < 0){
		berror("TH-ID %d: Receive failed! errcode - %d", ptData->iThreadID, errno);
		free(bscpPacket);
		ptData->ppiThreadTracker[ptData->iThreadID][1] = 0;
		sem_post(ptData->semThreadReady);
		return NULL;
	} 
	printf("TH-ID %d: BODY=%s\n", ptData->iThreadID, bscpPacket->szBody);

	/* Send response */
	if((iSent = send(
		ptData->sockClientDescriptor,
		bscpPacket->szBody,
		strlen(bscpPacket->szBody),
		MSG_DONTWAIT
	)) < 0){
		berror("TH-ID %d: SEND FAILED - errcode %d", ptData->iThreadID, errno);
		free(bscpPacket);
		ptData->ppiThreadTracker[ptData->iThreadID][1] = 0;
		sem_post(ptData->semThreadReady);
		return NULL;
	} 

	/* Simulate work */
	sleep(5);

	/* Signal completion to main process */
	free(bscpPacket);
	ptData->ppiThreadTracker[ptData->iThreadID][1] = 0;
	sem_post(ptData->semThreadReady);

	printf("TH-ID %d: THREAD COMPLETE! Exiting ... -> TRACKER [%d, %d, %d]\n",
		ptData->iThreadID,
		ptData->ppiThreadTracker[0][1],
		ptData->ppiThreadTracker[1][1],
		ptData->ppiThreadTracker[2][1]
	);

	/* TODO: Retrieve data from thread? */ 
	return NULL;
}
