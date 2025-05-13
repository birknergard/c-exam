#include <sys/socket.h>
#include <netinet/in.h> /* For INADDR_ANY */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 7575

/* Simple server for testing sending of created enc file */
int main(void){
   int sockServerDescriptor, sockNewDescriptor;
   int iNewAddressLength;
   int iStatusCode, iListened;
   int bBindSuccessful;

   /* If unchanged, return error free. */
   iStatusCode = 0;

   /* Open network socket (TCP/IP Protocol), as a stream */
   sockServerDescriptor = socket(AF_INET, SOCK_STREAM, 0);
   sockNewDescriptor = 0;

   if(sockServerDescriptor < 0){
      iStatusCode = errno;
   }

   /* Initialize server socket address to zero */
   struct sockaddr_in saServerAddress = {0}; 
   struct sockaddr_in saClientAddress = {0};

   /* Sets address type */
   saServerAddress.sin_family = AF_INET; 

   /* Converts port (int) into TCP/IP byte order (from little to big endian) */
   saServerAddress.sin_port = htons(PORT); 

   /* The server listens to any available network interfaces (wifi etc.) on the computer */
   saServerAddress.sin_addr.s_addr = INADDR_ANY;  
		  
   /* Bind socket to address */
   bBindSuccessful = bind(
      sockServerDescriptor,
      (struct sockaddr *) &saServerAddress,
      sizeof(saServerAddress)
   );

   /* Does not continue if bind failed */
   if(bBindSuccessful < 0){
      iStatusCode = errno;
      close(sockServerDescriptor); sockServerDescriptor = -1;
      return iStatusCode;
   } 

   /* Make server listen for input */
   iListened = listen(sockServerDescriptor, 2);
   if(iListened < 0){
	iStatusCode = errno;
	close(sockServerDescriptor); sockServerDescriptor = -1;
	return iStatusCode;
   }

   /* Initialize client socket address to zero */
   sockNewDescriptor = 0;
   iNewAddressLength = sizeof(saClientAddress); 

   /* Accept connection from client */
   sockNewDescriptor = accept(
      sockServerDescriptor,
      (struct sockaddr *) &saClientAddress,
      (socklen_t *) &iNewAddressLength
   );

   /* Does not continue if accept failed */
   if(sockNewDescriptor < 0){
      iStatusCode = errno;
      close(sockServerDescriptor); sockServerDescriptor = -1;
      close(sockNewDescriptor); sockNewDescriptor = -1;
      return iStatusCode;
   } 

   FILE *fpEncryptedFile = NULL;
   fpEncryptedFile = fopen("./selfmade.enc", "rb");
   if(fpEncryptedFile == NULL){
      close(sockServerDescriptor); sockServerDescriptor = -1;
      close(sockNewDescriptor); sockNewDescriptor = -1;
   }

   int iSize;
   if (fseek(fpEncryptedFile, 0, SEEK_END) == 0) {
      iSize = ftell(fpEncryptedFile);
      rewind(fpEncryptedFile);
   }

   char szBuffer[721] = {0}; 
   fread(szBuffer, iSize, 1, fpEncryptedFile);

   send(sockNewDescriptor, szBuffer, iSize, 0);

   close(sockServerDescriptor); sockServerDescriptor = -1;
   close(sockNewDescriptor); sockNewDescriptor = -1;
}
