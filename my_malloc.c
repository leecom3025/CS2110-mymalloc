#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you will receive a 20 point deduction. You have been
 * warned.
 */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)
#endif


/* make sure this always points to the beginning of your current
 * heap space! if it does not, then the grader will not be able
 * to run correctly and you will receive 0 credit. remember that
 * only the _first_ call to my_malloc() returns the beginning of
 * the heap. sequential calls will return a pointer to the newly
 * added space!
 * Technically this should be declared static because we do not
 * want any program outside of this file to be able to access it
 * however, DO NOT CHANGE the way this variable is declared or
 * it will break the autograder.
 */
void* heap;

/* our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * Technically this should be declared static for the same reasons
 * as above, but DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t* freelist[8];
/**** SIZES FOR THE FREE LIST ****
 * freelist[0] -> 16
 * freelist[1] -> 32
 * freelist[2] -> 64
 * freelist[3] -> 128
 * freelist[4] -> 256
 * freelist[5] -> 512
 * freelist[6] -> 1024
 * freelist[7] -> 2048
 */


void* my_malloc(size_t size)
{

	if ((long)size < 0) return NULL; // just for sure... 

	int sizeMem = size + sizeof(metadata_t);

	if (sizeMem > 2048)
	{	// too big request :( 
		ERRNO = SINGLE_REQUEST_TOO_LARGE;
		return NULL;
	}

	if (!heap) // first time
	{
		heap = my_sbrk(SBRK_SIZE); // initialize heap
		if (!heap)  // failure -> no memory :(
		{
			ERRNO = OUT_OF_MEMORY;
			return NULL; // break;
		}
		freelist[7] = (metadata_t *) heap;
		freelist[7]->in_use = 0;
		freelist[7]->size = 2048;
	}

	int index = getIndex(sizeMem);

	if(freelist[index]) { // easy case get the freelist block and manipulate the next and prev
		metadata_t* node = freelist[index];
		metadata_t* next = freelist[index]->next; // next temp
		metadata_t* prev = freelist[index]->prev; // prev temp
 
		index = getIndex(freelist[index]->size); // get the index

		if (prev && next) {
			prev->next = next; // next to prev's next
			next->prev = prev; // next's prev to prev
		} else if (prev && !next) {
			prev->next = NULL; // next is not there, make null
		} else if (!prev && next) {
			freelist[index] = next; // prev is not there, put next into freelist
			next->prev = NULL;
		} else {
			freelist[index] = NULL; // NEITHER next nor prev is there :( NULL to freelist[index]
		}

		node->next = NULL; // bye to my friends :) 
		node->prev = NULL;

		node->in_use = 1; // no
		ERRNO = NO_ERROR;
		return node + 1; // move pointer by one (Brandon gave me tip in Friday ;) 
	}

	//Get the next largest block of memory in the freelist
	int free = index;
	while(!freelist[free] && free <= 8) {
		free++;
	}

	//what if the free > 8 ?
	if (free >= 8) {
		metadata_t *mt = my_sbrk(SBRK_SIZE); // request more memory 
		if (mt == (void*) -1) // no more memory
		{
            		ERRNO = OUT_OF_MEMORY;
			return NULL;
		}

		mt->in_use = 0; //got fresh one
		mt->size = 2048;
		mt->prev = NULL; // just in case ;)
		mt->next = NULL;

		if(freelist[7]) // just in case
		{
			freelist[7]->next = mt;
		}else{
			freelist[7] = mt;
		}
		--free;
	}

	metadata_t *curr, *newptr;
	//free in the index is not avaliable, so go down blocks of memory until find one
	while(free != index) { // Think recursively!!! This is one of those algorithms that can be written much more easily using recursion.	
		curr = freelist[free]; // get the current block

		curr->size = ((curr->size)/2); //split into two blocks
		newptr = (metadata_t *) (((char*)curr) + curr->size); // move ptr by size
		newptr->size = curr->size; // size = size - metadata

		if(freelist[free]->next) { // if there is something in the next position
			freelist[free] = freelist[free]->next;
			freelist[free]->prev = NULL;
		} else { 		   // otherwise makes to NULL
			freelist[free] = NULL;
		}

		--free;	// decre index
		curr->next = newptr; // set current pointer's next to the new pointer
		newptr->prev = curr; // set new pointer's prev to the current pointer
		freelist[free] = curr; // put the current pointer into freelist[original free -1]
	}

	
	metadata_t *result = freelist[index];
	if(freelist[index]->next) {
		freelist[index] = result->next;
		freelist[index]->prev = NULL;
	} else {
		freelist[index] = NULL;
	}

	result->next = NULL;
	result->in_use = 1;
	ERRNO = NO_ERROR; // Good good good :)
	return result + 1; // move the pointer back to start block :)

}

void* my_calloc(size_t num, size_t size)
{
	// get approriate size of block by number of blocks * requested size
	void *mal = my_malloc(num * size);
	
	// if mal is null, the error is set in my_malloc function and simply returns null
	if (mal == NULL) {
		return NULL;
	}
	char *ptr = (char *) mal; // make temporary pointer
	char *end = ptr + (num * size); // make temporary end pointer that points to end of block
	while (ptr < end) { 
		*ptr = 0; // initialize to 0 
		ptr++; // move pointer 
	}
	ERRNO = NO_ERROR; // at this point we know it is no error b/c other errors handled by my_malloc
	return mal;
}

void my_free(void* ptr)
{
	if (ptr == NULL) return; // woops trying to free NULL? nonono 

	metadata_t *freeptr = (metadata_t *)((char *)ptr - sizeof(metadata_t)); // go to metadata part
	metadata_t *mybuddy = (metadata_t *) findBuddy(freeptr); //get my buddy XD


	if(!mybuddy && !freeptr->in_use) { // my buddy is there but I already got freed
		ERRNO = DOUBLE_FREE_DETECTED; // DOUBLE FREE!
		return;
	}

	freeptr->in_use = 0; // set the block's in_use variable to 0

	if(mybuddy && !mybuddy->in_use && mybuddy->size != SBRK_SIZE) {
		swap(freeptr, mybuddy); // swap it
		removeFromList(mybuddy); // remove my buddy from list
		removeFromList(freeptr); // remove me from list
		metadata_t *p; // merge two blocks
		if (freeptr > mybuddy) // assign p to whatever the smaller one is 
			p = mybuddy;
		else 
			p = freeptr;

		p->size *= 2; // double the size

		int index = getIndex(p->size);
		metadata_t *curr = freelist[index]; // get freelist 
		if (curr) { 
			//get the tail
			while(curr->next) {
				curr = curr->next;
			}
			curr->next = p;
			p->prev = curr;
			p->next = NULL;
		} else {
			freelist[index] = p;
		}

		if (findBuddy(p) && p->size != SBRK_SIZE) { // if merged one's buddy is there 
			my_free((char *) p + sizeof(metadata_t)); // free p 
		} else {
			ERRNO = NO_ERROR; // NO error and bye bye 
			return;
		}

	} else {
		// I Don't have buddy :(((((
		// same logic here except I use free pointer, not p
		int index = getIndex(freeptr->size); 
		metadata_t *curr = freelist[index];
		if (curr) {
			//get the tail
			while(curr->next) {
				curr = curr->next;
			}
			curr->next = freeptr;
			freeptr->prev = curr;
			freeptr->next = NULL;
		} else {
			freelist[index] = freeptr;
		}
		ERRNO = NO_ERROR;
		return;
	}
}

void* my_memmove(void* dest, const void* src, size_t num_bytes)
{
	if (dest == src) { // dest equal to src ? bye bye 
		ERRNO = NO_ERROR; // no error of course
		return dest;
	}

	char *d = (char *)dest; // temp d
	char *s = (char *)src; // temp s 
	int i = 0;

	if (d > s) { // if d is bigger
		i = num_bytes -1; // last index
		while(i >= 0) { 
			d[i] = s[i]; // move move 
			i--; // move index toward front
		}
	} else { // s is bigger
		while(i < num_bytes) { // woops i is smaller
			d[i] = s[i]; // move move
			i++; // move index toward back
		}
	}
	ERRNO = NO_ERROR; // no error
	return dest; // return dest
}


int getIndex(size_t size) {
	int index = 0; // starting from 0
	int memory = 16; // base memory 
	while (memory < size) { // if memory is still smaller than requested size
		memory *= 2; // double it
		index++; // increment index
	}
	return index;
}

metadata_t* removeFromList(metadata_t* node)
{
	metadata_t* next = node->next; // temp next
	metadata_t* prev = node->prev; // temp prev

	int index = getIndex(node->size); // get the index of node's size

	if (prev && next) {
		prev->next = next; // next to prev's next
		next->prev = prev; // next's prev to prev
	} else if (prev && !next) {
		prev->next = NULL; // next is not there, make null
	} else if (!prev && next) {
		freelist[index] = next; // prev is not there, put next into freelist
		next->prev = NULL;
	} else {
		freelist[index] = NULL; // NEITHER next nor prev is there :( NULL to freelist[index]
	}

	node->next = NULL; // NULL
	node->prev = NULL;

	return node;
}


void swap(metadata_t* a, metadata_t* b)
{
	// Swap them :) straightforward logic
	if (a->next == b) 
		a->next = b->next;
	if (a->prev == b) 
		a->prev = b->prev;
	if (b->next == a) 
		b->next = a->next;
	if (b->prev == a) 
		b->prev = a->prev;
}

/*
 Find the buddy of the given block.
 Using XOR operator b + or - (1 << log_2(size))
 */
metadata_t *findBuddy(metadata_t *b) {
	long mybuddy = (long)b ^ (long)b->size;
	metadata_t *result = (metadata_t *) mybuddy;

	if (b->size == result->size) { // making sure they have same size
		 return result;
	}
	return NULL;
}





