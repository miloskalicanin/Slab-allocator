#ifndef _BUDDY_H_
#define _BUDDY_H_

void buddy_init(void *space, int block_num);	//allocate buddy

void* buddy_alloc(int n);	//allocate page (size of page is 2^n)

void buddy_free(void *space, int n);	//free page (starting address is space, size of page is 2^n)

void buddy_print();		//print current state of buddy


#endif