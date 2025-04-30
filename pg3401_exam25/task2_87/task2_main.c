/* TITLE: TASK 2 MAIN SOURCE 
 * AUTHOR: 87
 * DESCRIPTION: Main executable responsible for parsing data into given metadata struct,
 *  as well as converting said struct into binary output file.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task2_count.h"
#include "task2_hash.h"
#include "task2_sum.h"

#include "task2_main.h"

unsigned int main(void){
   puts("PROGRAM START!\n");
   puts("1. Declaring variables\n");
   char *pszBuffer = NULL;
   FILE *fSource = NULL; 
   FILE *fOutput = NULL;
   struct TASK2_FILE_METADATA *pfmetaData = NULL;
   
   puts("2. Allocating to metadata struct\n");
   pfmetaData = (struct TASK2_FILE_METADATA *) malloc(sizeof(struct TASK2_FILE_METADATA));
   memset(pfmetaData, 0, sizeof(struct TASK2_FILE_METADATA));

   /* Opening source txt file with read permission */
   puts("3. Opening pgexam25_test.txt\n");
   fSource = fopen("./pgexam25_test.txt", "r");

   /* Performing functions and storing the results in metadata struct */
   puts("4. Executing functions and storing data to metadata\n");
   Task2_CountEachCharacter(fSource, pfmetaData->aAlphaCount);
   Task2_SimpleDjb2Hash(fSource, (unsigned int *) pfmetaData->byHash);
   Task2_SizeAndSumOfCharacters(fSource, &pfmetaData->iFileSize, &pfmetaData->iSumOfChars);

   /* Closing source text file */
   puts("5. Closing pgexam25_test.txt\n");
   fclose(fSource);
   fSource = NULL;

   /* Opening output file with write permission (binary)*/
   puts("5. Opening and writing metadata to pgexam25_output.bin (in binary)\n");
   fOutput = fopen("./pgexam25_output.bin", "wb");
   fwrite(pfmetaData, sizeof(struct TASK2_FILE_METADATA), 1, fOutput);

   puts("6. Closing pgexam25_output.bin and freeing metadata struct ...\n");
   /* Closing output file */
   fclose(fOutput);
   fOutput = NULL;

   /* Freeing metadata pointer */
   free(pfmetaData);

   puts("PROGRAM COMPLETE! EXITING ...\n");
   return 0;
}
