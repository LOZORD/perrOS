#ifndef MYMEM_H
#define	MYMEM_H

void * Mem_Init(int sizeOfRegion, int slabSize);
void * Mem_Alloc(int size);
int Mem_Free(void *ptr);
void Mem_Dump();

struct FreeHeader
{
	int length;
	struct FreeHeader * next;
};

struct AllocatedHeader
{
	int length;
	void * magic;	// Trust me.
};

#define	MAGIC	0x5077604

#endif // MYMEM_H
