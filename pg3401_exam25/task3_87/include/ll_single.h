#ifndef ___LL_SINGLE___

#define ___LL_SINGLE___

#pragma pack (1)
/*
 * The struct NODE containts a pointer to data,
 * as well as a pointer to the "next" node.
 * */
typedef struct _NODE {
    struct _NODE *pNext;
	    void *pData; /* Creates an generic pointer to the data instead of holding the data itself*/
} NODE;
#pragma pack ()

#pragma pack (1)
/*
 *  The struct LIST is a linked list implementation that holds
 *  the head of the linked list, as well as the total length.
 * */
typedef struct _LL_SINGLE{
    NODE *noHead;
    char *szType;
    int iLength;
} LL_SINGLE;
#pragma pack ()

/*
 * This is the constructor function for a NODE struct. It takes a generic (or void)
 * pointer as an argument, which it contains within itself.
 * Returns a memory adress containing a NODE struct.
 * */
static NODE *_NewNode(void *pData);

/*
 * This is the constructor function for a LIST struct. It takes a generic (or void)
 * pointer as an argument, which it stores within the head node.
 * Returns a memory adress containing a LIST struct.
 * */
LL_SINGLE *NewList();

/*
 *  This function pushes a node into the linked list, making it the new head.
 * */
void Push(LL_SINGLE *lip, void *pData);

/*
 *  This function appends a node to the end of the list.
 * */
void Append(LL_SINGLE *lip, void *pData);


/*
 *  This function retrieves the data of a node given its position (0 indexing) in the list.
 *  This data is (for now) retrieved in the form of a void pointer which needs to be
 *  explicitly cast and dereferenced manually. For example, given a linked list "L":
 *
 *  int data = *(int) Get(LINKEDLIST, INDEX);
 * */
void *GetValue(LL_SINGLE *lip, int iIndex);


static NODE *_GetNode(LL_SINGLE *lip, int iIndex);

int RemoveFirst(LL_SINGLE *lip);  

int RemoveLast(LL_SINGLE *lip);  


#endif /*ndef ___LL_SINGLE___ */
