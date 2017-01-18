#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>



#define SYSTEM_MALLOC 1
#define SBRK_SIZE 4096
typedef struct blocks block; 
typedef struct footer foot;

struct blocks {
	size_t size;
	int free;
	block *next;
	block *footer;
};


block *extend_heap(block *head, int size);
block *coalasce(block *ptr);

extern void *mymalloc(unsigned int size);
/* mymalloc: allocates memory on the heap of the requested size. The block
             of memory returned should always be padded so that it begins
             and ends on a word boundary.
     unsigned int size: the number of bytes to allocate.
     retval: a pointer to the block of memory allocated or NULL if the 
             memory could not be allocated. 
             (NOTE: the system also sets errno, but we are not the system, 
                    so you arvoid *mymalloc(unsigned int size)e not required to do so.)
*/
block *head;
block *tmp = NULL;
pthread_mutex_t lock;
block *start_heap;

int align_word(unsigned int size){
		float word = ceil((float) size/4);
		int n = (word*4);		
		return n;
	}

void *mymalloc(unsigned int size) {
	pthread_mutex_lock(&lock);
	char *p;
	block* hd;
	//if the head is NULL initalize it
	if(!tmp){
		block* footer;
		p = sbrk(SBRK_SIZE);
		tmp = (block *)p;
		footer = (block *) (p+(SBRK_SIZE-sizeof(block))); 
		footer->free = 1;
		footer->size = SBRK_SIZE;
		footer->footer = tmp;
		footer->next = NULL;
		tmp->footer = footer;
		tmp->size = SBRK_SIZE;	
		tmp->free = 1;
		tmp->next = NULL;
		start_heap = tmp;

	}

	//calculate the size of memory needed to allocate
	int mem_needed = align_word(size + 2*sizeof(block));

	if(tmp->size >= mem_needed &&  tmp->free){
		// if the remaining size is greater than dataoffset then split block
		if(tmp->size - mem_needed > 2*sizeof(block)){
		//	printf("sz %p\n", tmp + (sizeof(int) + sizeof(block)));			
			block* footer;
			hd = (void *) tmp + mem_needed;
			footer = (void *) tmp + (mem_needed - sizeof(block));
			footer->free = 1;
			hd->size = tmp->size - mem_needed;
			footer->footer = tmp;
			footer->next = NULL;
			hd->footer = tmp->footer;
			hd->footer->size = hd->size;
			hd->footer->free = 1;
			hd->footer->footer = hd;
			tmp->size = mem_needed;
			hd->free = 1;
			tmp->next = hd;
			footer->size = mem_needed;
			tmp->footer = footer;

			//printf("parca %p, %p\n",tmp, tmp->footer);
			//printf("parca1 %p, %p\n",hd, hd->footer);
		}
		//create the new block to return
		tmp->free = 0;
		tmp->footer->free = 0;
		head = tmp;
		tmp = tmp->next;
	//	printf("return %p",hd);
		pthread_mutex_unlock(&lock);
		head->free = 0;
		head->footer->free = 0;
		char *ptr = (char *)head;
		ptr = ptr + (short)sizeof(block);
		block *result = (block *)ptr;
		pthread_mutex_unlock(&lock);

		//printf("result %p\n",result );
		return result;
	}

	head = tmp;
	/*loop the list to find first block that has equal or greater than memory we need*/	
	while((head->size < mem_needed ||(!head->free)) && (head->next)){
		head = head->next;
	}

	if((!head)|| head->size < mem_needed){
		float n = ceil((float) mem_needed/SBRK_SIZE);
		int mem = SBRK_SIZE*n;
		p = sbrk(mem);
	//	printf("sbrk%p\n",p );
		block *footer;
		block *a = (void *)p;
		//start_heap = a;
	//	printf("start_heap %p\n", start_heap);
		a->size = mem;
		a->free = 1;
		a->next = NULL;
		footer = (void *) (p + (mem -sizeof(block)));
		footer->footer = a;
		footer->next = NULL;
		footer->size = a->size;
		a->footer = footer;
		head->next = a;
	//	printf("head %p\n",head );
		head = coalasce(head);

		//head = head->next;
	//	printf("haed %p, %p\n",head, head->footer);
	}

	if(head->size - mem_needed > 2*sizeof(block)){

		block* footer;
		hd = (void *) head + mem_needed;
		footer = (void *) head+ (mem_needed - sizeof(block));
		footer->free = 1;
		hd->size = head->size - mem_needed;
		footer->footer = tmp;
		footer->next = NULL;
		hd->footer = head->footer;
		hd->footer->size = hd->size;
		hd->footer->free = 1;
		hd->footer->footer = hd;
		head->size = mem_needed;
		hd->free = 1;
		head->next = hd;
		footer->size = mem_needed;
		head->footer = footer;

		//printf("mem_needed %p\n",hd );			
	}
	
	head->free = 0;
	head->footer->free = 0;
	char *ptr = (char *)head;
	ptr = ptr + (short)sizeof(block);
	block *result = (block *)ptr;
		pthread_mutex_unlock(&lock);

	return result;

	
	

}

/*it extends the heap if requested size is bigger than SBRK_SIZE
it takes head of the list and the size how much to extend as parameters*/
block *extend_heap(block *head, int size){
	char *p = sbrk(SBRK_SIZE*size);
	block *a = (void *) p;
	block *footer;
	footer = (void *) (p + (SBRK_SIZE*size - sizeof(block)));
	//printf("extend %d",SBRK_SIZE*size);
	a->size = SBRK_SIZE*size;
	a->free = 1;
	a->next = head;
	footer->size = a->size;
	footer->free = 1;
	footer->footer = a;
	footer->next = NULL;
	a->footer = footer;
	head = a;
	return a;

}

/* merge the blocks if its neighbors are free*/
block *coalasce(block *ptr){
	block *footer;
	footer = (block *) (ptr - 1);
	if(ptr!=start_heap){

	if(footer->free){

		block *new = footer->footer;
		new->size+= ptr->size;
		new->footer = ptr->footer;
		new->footer->size = new->size;
		new->footer->footer = new;
		new->next = ptr->next;
			
}}
	/*printf("yoxla %p,%d,%p\n", footer, footer->size, footer->footer);
	printf("yoxla2 %p,%d,%p\n", new, new->size, new->footer);*/

	if(ptr->next){
		if(ptr->next->free){
			ptr->size += ptr->next->size;
			ptr->footer = ptr->next->footer;
			ptr->next = ptr->next->next;
			ptr->footer->size = ptr->size;	
	}}
	return ptr;
}
/* myfree: unallocates memory that has been allocated with mymalloc.
     void *ptr: pointer to the first byte of a block of memory allocated by 
                mymalloc.
     retval: 0 if the memory was successfully freed and 1 otherwise.
             (NOTE: the system version of free returns no error.)
*/
void myfree(void *ptr) {
	pthread_mutex_lock(&lock);
	char *p = (char *)ptr;
	p = p - sizeof(block);
	block *a = (block *)p;
	if(a != tmp){
		a->next = tmp;
		tmp = a;}
	if(!ptr){
		printf("Trying to free freed pointer");
		pthread_mutex_unlock(&lock);
		return;
	}
		tmp->free = 1;
		tmp->footer->free =1;
		//printf("check before coalasce %p, size = %d\n", head, head->size);
		tmp = coalasce(tmp);
	//	printf("tmp 	%p\n",tmp );
		//printf("check after coalasce %p, size = %d\n", head->next, head->size);
	pthread_mutex_unlock(&lock);
}

/*int main(){
		int *p = mymalloc(2040);
		printf("mymalloc(%d) %p\n",2040, p);
		//myfree(p);
		int *a = mymalloc(2040);
		printf("mymalloc(%d) %p\n",2040, a);
		myfree(a);
		int *t = mymalloc(48);
		printf("mymalloc(%d) %p\n",48, t);
		//myfree(t);
		int *z = mymalloc(4072);
		printf("mymalloc(%d) %p\n",4072, z);
		//myfree(z);
		int *e = mymalloc(4072);
		printf("mymalloc(%d) %p\n",4072, e);
		//myfree(e);
		int *q = mymalloc(4072);
		printf("mymalloc(%d) %p\n",4072, q);
		//myfree(q);
		tmp = start_heap;
	//	printf("start_heap %p\n", start_heap);
		while(tmp!=NULL){
			printf("tmp %p, %d, free %d\n", tmp, tmp->size, tmp->free);
			tmp = (void *)tmp + tmp->size;	
}
		

	return 0;


}*/
