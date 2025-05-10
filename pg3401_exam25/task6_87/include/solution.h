#ifndef SOLUTIION_H
#define SOLUTION_H

#define MAX_HTTP_HEADER 8192 /* Apache default limit */
#define MAX_HTTP_RESPONSE 4096

#define BYTE unsigned char
#define BY4 unsigned int
#define BY8 unsigned long

#pragma pack(1)
typedef struct _ENC_REQUEST{
   char *szHeader;
   BY8 *arrlEncrypted;
} ENC_REQUEST;
#pragma pack()

union UN_BY8 {
   BY4 by4First;
   BY4 by4Second;
   BY8 by8Normal;
};

#endif /*ndef SOLUTION_H*/
