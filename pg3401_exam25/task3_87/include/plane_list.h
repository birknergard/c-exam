/*--------------------------------------------------
* AUTHOR: BKN
* DESCRIPTION: Doubly linked list implementation
*              which holds anonymous datatype in
*              form of a void *. Hence it is made
*              to be used as a baseline for further
*              abstractions.
* PREFIX: lp (For LIST of Planes)
--------------------------------------------------*/
#include "passenger_list.h"

#ifndef ___PLANE_LIST_H___ 
#define ___PLANE_LIST_H___  

#define ID_SIZE 4

#pragma pack(1)

typedef struct _PLANE_DATA{
   char szFlightID[ID_SIZE]
   int iSeats;
   int iDepartureTime;
   PASSENGER_LIST plPassengers;
}

#pragma pack()
/* Node struct */
typedef struct _PLANE {
   struct _PLANE *pNext;
   struct _PLANE *pPrev;
   int iSize;
   PLANE_DATA *pdData;
} PLANE;

/* Doubly linked list struct */
typedef struct _PLANE_LIST {
   PLANE *pHead;
   PLANE *pTail;
   int iPlanes;
} PLANE_LIST;

/* Creation functions */
PLANE_LIST *CreatePlaneList();
int DestroyPlaneList(PLANE_LIST **pplpList);

/* Getter functions */
PLANE *GetPlane(PLANE_LIST lp, int i);
void *GetData(PLANE_LIST lp, int i);

/* List modification functions - return OK or ERROR */
int AddPlane(PLANE_LIST *plp, PLANE_DATA dpData);

int RemovePlane(PLANE_LIST *plp, PLANE *pPlane);

#endif /*ndef ___LL_DOUBLE_H___  */ 
