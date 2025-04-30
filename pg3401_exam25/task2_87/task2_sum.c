/* This file has been created by EWA, and is part of task 2 on the exam for PG3401 2025*/

#include <stdio.h> /* I attempted to put this in the header file (to avoid changing this file) but it did not work */
#include "task2_sum.h"

int Task2_SizeAndSumOfCharacters(FILE* fFileDescriptor, int* piSizeOfFile, int* piSumOfChars) {
   fseek(fFileDescriptor, 0, SEEK_END);
   *piSizeOfFile = ftell(fFileDescriptor);
   rewind(fFileDescriptor);
   *piSumOfChars = 0;
   for (int i = 0; i < *piSizeOfFile; i++){ *piSumOfChars += fgetc(fFileDescriptor); }
   rewind(fFileDescriptor);
   return 0;
}
