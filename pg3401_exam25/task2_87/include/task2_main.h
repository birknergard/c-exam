/* TASK 2 HEADER FILE
 * AUTHOR: 87
 * DESCRIPTION: Header file for main program for this submission.
 * */
#ifndef ___TASK2_MAIN_H___
#define ___TASK2_MAIN_H___

#include <stdio.h>

/* Defines the struct required for the assignment */
struct TASK2_FILE_METADATA {
	char szFileName[32];
	int iFileSize;
	char byHash[4];
	int iSumOfChars;
	char aAlphaCount[26];
}

#endif /* ___TASK2_MAIN_H___  */
