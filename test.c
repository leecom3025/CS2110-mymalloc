#include "my_malloc.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
// Joseph Print XD
#define jp(x) printf(x"\n");
#define jpa(x, args ) printf(x"\n", args);

extern metadata_t* freelist[8];



/* debug */
void print_block(metadata_t *block) {
  fprintf(stderr, "Printing block data\n");
  fprintf(stderr, "address: %p\n", (void *) block);
  fprintf(stderr, "in use: %d\n", block->in_use);
  fprintf(stderr, "size: %d\n", block->size);
  fprintf(stderr, "next: %p\n", (void *) block->next);
  fprintf(stderr, "prev: %p\n", (void *) block->prev);
  fprintf(stderr, "\n");
}

void print_freelist() {
	int size = 16;
	void *a[8];
	for (int i=0; i<8; i++) {
		fprintf(stderr, "[%d] -> %d: %p\n", i, size, (void *) freelist[i]);
		if (freelist[i]) {
			a[i] = freelist[i];
		}
		size *= 2;
	}
	printf("\n");
	
	for(int i = 0; i < 8; i++) {
		if (freelist[i] != NULL)
			print_block(a[i]);
	}
}

/*debug*/
void print_ll(LIST *list) {
	if (list == NULL || list->size == 0) {
		printf("List is empty\n");
		return;
	}

	printf("Printing\n");
	NODE *curr = list->head;
	printf("Head ");
	int i = 0;
	while(curr){
		printf("-> %d(%d)", curr->data,i);
		curr = curr->next;
		i++;
	}
	printf(" <- tail\n");
}



void debug() {
	srand(time(NULL));
	LIST *ll = my_malloc(sizeof(LIST));
	for (int i = 0; i < 1500; i++) {
		int elem = rand() % 2036;

		printf("%d. Adding %d\n", i, elem);
		pushFront(ll, elem);
		//if (ERRNO != NO_ERROR)
		//	break;

	}
	printf("Printing=====\n");

	print_ll(ll);
}

void bdebug() {
	srand(time(NULL));
	void* arr[50];
	int NUM = 50;
	int total = 0;
	for (int i = 0; i < NUM; i++) {
		int elem = rand() % 500;
		arr[i] = my_malloc(elem);
		total += elem;
		printf("%d. Adding %d , total now %d\n", i, elem, total);

		if (ERRNO != NO_ERROR) {
			print_freelist();
			jpa("Error (OUT_OF_MEMORY) 1 =? %d", ERRNO);
			break;		
		}

	}
	printf("=============================Total: %d Bytes", total); // 8192
	print_freelist();
	printf("Malloc worked\n");
	for (int i = 0; i < NUM; i++) {
		//printf("%d Free\n", i);
		if (!arr[i])
		my_free(arr[i]);
	}
	print_freelist();
	printf("Free worked\n");
	return;
}

void pb(metadata_t *block) {
	// move the pointer to the back to metadata
	block = ((metadata_t*)((int)block - sizeof(metadata_t)));
	fprintf(stderr, "Printing block data\n");
	fprintf(stderr, "address: %p\n", (void *) block);
	fprintf(stderr, "in use: %d\n", block->in_use);
	fprintf(stderr, "size: %d\n", block->size);
	fprintf(stderr, "next: %p\n", (void *) block->next);
	fprintf(stderr, "prev: %p\n", (void *) block->prev);
	fprintf(stderr, "\n");
}


void jd() {
	jp("metadata->inuse after allocating?");
	int test = 50;
	void* x = my_malloc(sizeof(test));
	pb(x);
	my_free(x);

	jp("request size: 1012, expected size of 1024");
	x = my_malloc(1012);
	pb(x);
	my_free(x);

	jp("request size: 1024, expected size of 2048");
	x = my_malloc(1024);
	pb(x);
	my_free(x);
	
	jpa("request size: MB_LEN_MAX, expected size of %d", MB_LEN_MAX);
	x = my_malloc(MB_LEN_MAX);
	pb(x);
	my_free(x);

	jpa("request size: 2036, expected size of %d", 2036);
	x = my_malloc(2036);
	pb(x);
	jpa("NO ERROR: %d\n",ERRNO);
	my_free(x);

	jp("request size: 2037, expected SINGLE_REQUEST_TOO_LARGE(2)");
	x = my_malloc(2037);
	jpa("%d\n",ERRNO);

	jp("request size: -1, expected Some error");
	x = my_malloc(-1);
	if (x == NULL) {
		jp("received NULL block");
	} else {
		jp("received some weird one:(");
		pb(x);	
		my_free(x);
	}

/* my_free test case */
	void *t1,*t2,*t3,*t4;
	jp("\n\n========= my_free test cases ==========");
	jp("the in_use field should be set to 0 when freed");
	int myfree = 500; 
	x = my_malloc(myfree);
	pb(x);
	my_free(x);
	pb(x);

	jp("Allocating 500 for three times.\nBefore:");
	print_freelist();
	t1 = my_malloc(myfree);
	t2 = my_malloc(myfree);
	t3 = my_malloc(myfree);
	
	jp("After:");
	print_freelist();

	jp("One more time:");
	t4 = my_malloc(myfree);

	jp("Expected no elements at all");
	print_freelist();

	jp("");
	my_free(t1);
	print_freelist();

	jp("");
	my_free(t2);
	print_freelist();

	jp("");
	my_free(t3);
	print_freelist();

	jp("");
	my_free(t4);
	print_freelist();

	my_free(t4);
	jp("my_free(t4) double , expected DOUBLE_FREE_DETECTED(3)");
	jpa("The ERROR is %d", ERRNO);


/* my_calloc test */
	jp("\n\n========= my_free test cases ==========");
	x = my_calloc(myfree, 4);
	jp("Expected no elements at all");
	print_freelist();
	
	jp("Move the pointer to 1st");
	t1 = (metadata_t*)(((char*)x)); 
	pb(t1);
	my_free(x);
	
	print_freelist();
	jp("");
	x = my_calloc(myfree, 2);
	jp("my_calloc(myfree, 2), expected NO_ERROR(0)");
	jpa("The ERROR is %d", ERRNO);

	jp("Should have 1024 size block left");
	print_freelist();
	my_free(x);

	jp("Requesting myfree*6 = 3000");
	x = my_calloc(myfree, 6);
	jp("my_calloc(myfree, 6), expected SINGLE_REQUEST_TOO_LARGE(2)");
	jpa("The ERROR is %d\n", ERRNO);
	my_free(x); 

	jp("Back to NO_ERROR");
	x = my_calloc(myfree, 4);
	jp("my_calloc(myfree, 2), expected NO_ERROR(0)");
	jpa("The ERROR is %d\n", ERRNO);
	

	jp("Now requesting over heap");
	t1 = my_calloc(myfree, 4);
	t2 = my_calloc(myfree, 4);
	t3 = my_calloc(myfree, 4);
	t4 = my_calloc(myfree, 4); 
	// total of 2012 * 5 = 10012 -> over heap
	jp("my_calloc(myfree, 6), expected OUT_OF_MEMORY(1)");
	jpa("The ERROR is %d", ERRNO);
	my_free(t1);
	my_free(t2);
	my_free(t3);
	my_free(t4);
	my_free(x);


/* OUT OF MEMORY case confirmed by Prof. Leahy XD */
	jp("Before asking for 2000 bytes 4 tiems");
	print_freelist();

	t1 = my_malloc(2000);
	t2 = my_malloc(2000);
	t3 = my_malloc(2000);
	t4 = my_malloc(2000);

	jp("After asking for 2000 bytes 4 times");
	print_freelist();
	jp("Expected error is NO_ERROR(0)");
	jpa("The ERROR is %d\n", ERRNO);

	x = my_malloc(1);
	jp("x = my_malloc(1) after 4 times of my_malloc(2000), expected OUT_OF_MEMORY(1)");
	jpa("The ERROR is %d", ERRNO);
/* Some random cases */
	jp("Trying to free NULL");
	my_free(NULL);
	jp("NULL is not freed :)");

}

/* Linked List test */
void lt() {
	srand(time(NULL));
	pushFront(NULL, 5);
	
	LIST *ll = my_malloc(sizeof(LIST));
	for (int i = 0; i < 20; i++) {
		int elem = rand() % 2036;
		printf("%d. Adding %d\n", i, elem);
		pushFront(ll, elem);

	}
	printf("Printing=====\n");

	print_ll(ll);

	pushBack(ll, (int)NULL);
	pushBack(ll, 11);
	print_ll(ll);
	jpa("Size of ll: %d elements", ll->size);

	int x = popFront(ll);
	print_ll(ll);
	printf("Element %d\n", x);
}


int main() {
	// Test my_malloc, my_free normal conditions

	//printf("=======Brandon Debug========\n");
	bdebug();

	//debug();
	//jd();
	//printf("=======Linked List Debug========\n");
	//lt();
	//print_freelist();
		
	//metadata_t *k = my_malloc(1000000000);
	//print_freelist();
	//my_free(k);
	//print_freelist();
	// O.o......
	return 0;
}


