#include "buddy.h"
#include "slab.h"
#include <iostream>
#include <ctgmath>
using namespace std;


void* buddySpace;
int numOfEntries;
int startingBlockNum;

void** freeList;




void buddy_init(void *space, int block_num)
{
	if (space == nullptr || block_num < 2) exit(1); //broj blokova mora biti veci od 1 (1 blok odlazi na buddy)

	startingBlockNum = block_num;
	buddySpace = space;
	space = ((char*)space + BLOCK_SIZE);	//ostatak memorije se koristi za alokaciju
	block_num--;	//prvi blok ide za potrebe buddy alokatora
	
	int i = 1;
	numOfEntries = log2(block_num) + 1;

	freeList = (void**)buddySpace;
	for (i = 0; i < numOfEntries; i++)
		freeList[i] = nullptr;

	int maxLength = numOfEntries-1;
	int maxLengthBlocks = 1 << maxLength;

	while (block_num > 0)
	{
		void* addr = (char*)space + (block_num - maxLengthBlocks)*BLOCK_SIZE;
		freeList[maxLength] = addr;
		*(void**)addr = nullptr;
		block_num -= maxLengthBlocks;
		
		if (block_num > 0)
		{
			i = 1;
			maxLength = 0;
			while (true)
			{
				if (i <= block_num && 2 * i > block_num)
					break;
				i = i * 2;
				maxLength++;
			}
			maxLengthBlocks = 1 << maxLength;
		}
	}
}



void* buddy_alloc(int n)
{
	if (n < 0 || n >= numOfEntries)	return nullptr;

	void* returningSpace = nullptr;

	if (freeList[n] != nullptr)
	{
		returningSpace = freeList[n];
		freeList[n] = *(void**)returningSpace;
		*(void**)returningSpace = nullptr;
	}
	else
	{
		for (int i = n+1; i < numOfEntries; i++)
		{
			if (freeList[i] != nullptr)
			{
				void* ptr1 = freeList[i];
				freeList[i] = *(void**)ptr1;
				void* ptr2 = (char*)ptr1 + BLOCK_SIZE*(1 << (i - 1));

				*(void**)ptr1 = ptr2;
				*(void**)ptr2 = freeList[i-1];
				freeList[i - 1] = ptr1;

				returningSpace = buddy_alloc(n);
				break;
			}
		}

	}

	return returningSpace;
}


inline bool isValid(void* space, int n)		//check if starting address (space1) is valid for length 2^n
{
	int length = 1 << n;
	int num = ((startingBlockNum-1)%length) + 1;
	int i = ((char*)space - (char*)buddySpace) / BLOCK_SIZE;	//num of first block

	if (i%length == num%length)		//if starting block number is valid for length 2^n then true
		return true;

	return false;
}


void buddy_free(void *space, int n)
{
	if (n < 0 || n >= numOfEntries)	return;

	int bNum = 1 << n;

	if (freeList[n] == nullptr)
	{
		freeList[n] = space;
		*(void**)space = nullptr;
	}
	else
	{
		void *prev = nullptr;
		void* curr = freeList[n];
		while (curr != nullptr)
		{
			if ( curr == (void*)( (char*)space + BLOCK_SIZE*bNum) )		//right buddy potentially found
			{
				if (isValid(space,n+1))		//right buddy found
				{
					if (prev == nullptr)
					{
						freeList[n] = *(void**)freeList[n];
					}
					else
					{
						*(void**)prev = *(void**)curr;
					}

					buddy_free(space, n + 1);

					return;
				}
			}
			else if ( space == (void*)((char*)curr + BLOCK_SIZE*bNum) )	//left buddy potentially found
			{
				if (isValid(curr,n+1))		//left buddy found
				{
					if (prev == nullptr)
					{
						freeList[n] = *(void**)freeList[n];
					}
					else
					{
						*(void**)prev = *(void**)curr;
					}

					buddy_free(curr, n + 1);

					return;
				}
			}

			prev = curr;
			curr = *(void**)curr;
		}
		
		*(void**)space = freeList[n];
		freeList[n] = space;
	}
}



void buddy_print()
{
	cout << "Buddy current state (first block,last block):" << endl;
	for (int i = 0; i < numOfEntries; i++)
	{
		int size = 1 << i;
		cout << "entry[" << i << "] (size "<< size << ") -> ";
		void* curr = freeList[i];
		
		while (curr != nullptr)
		{
			int first = ((char*)curr - (char*)buddySpace) / BLOCK_SIZE;
			cout <<"("<< first << "," << first + size - 1 << ") -> ";
			curr = *(void**)curr;
		}
		cout << "NULL" << endl;
	}
}



