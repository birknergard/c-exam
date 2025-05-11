#ifndef SOLUTIION_H
#define SOLUTION_H

#define MAX_HTTP_HEADER 8192 /* Apache default limit */
#define MAX_HTTP_RESPONSE 4096

#define BYTE unsigned char
#define BY4 unsigned int
#define BY8 unsigned long

union UN_BY8 {
   char schar[8];
   BYTE by[8];
   BY4 by4[2];
   BY8 by8Base;
};

#pragma pack(1)
typedef struct _ENC_REQUEST{
   char *szHeader;
   union UN_BY8 *arrby8Encrypted;
} ENC_REQUEST;
#pragma pack()


#endif /*ndef SOLUTION_H*/
