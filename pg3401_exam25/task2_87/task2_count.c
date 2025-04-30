/* This file has been created by EWA, and is part of task 2 on the exam for PG3401 2025*/

#include <stdio.h> /* I attempted to put this in the header file (to avoid changing this file) but it did not work */
#include "task2_count.h"

int Task2_CountEachCharacter(FILE * fFileDescriptor, char aCountArray[26]) {
   int iCharacter = 0; 
   rewind(fFileDescriptor); 
   do {
      iCharacter = fgetc(fFileDescriptor); 
      if (iCharacter >= 'a' && iCharacter <= 'z') {
         aCountArray[iCharacter - 'a']++; 
      }
      else if (iCharacter >= 'A' && iCharacter <= 'Z') {
         aCountArray[iCharacter - 'A']++; 
      }
   } while (iCharacter != EOF); 
   rewind(fFileDescriptor); 
   return 0; 
}
