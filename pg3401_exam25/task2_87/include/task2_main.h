/* TASK 2 HEADER FILE
 * AUTHOR: 87
 * DESCRIPTION: Header file for main program for this submission.
 * */
#ifndef ___TASK2_MAIN_H___
#define ___TASK2_MAIN_H___

/* Defines the struct required for the assignment, with 1 byte packing to prevent unexpected size. */
#pragma pack (1)
struct TASK2_FILE_METADATA {
	char szFileName[32];
	int iFileSize;
	char byHash[4];
	int iSumOfChars;
	char aAlphaCount[26];
};
#pragma pack ()

#endif /* ___TASK2_MAIN_H___  */
