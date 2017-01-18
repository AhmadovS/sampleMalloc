typedef struct blocks block; 

struct blocks {
	size_t size;
	int free;
	block *next;
};
block *extend_heap(block *head, int size);
void split_block(block *b, int sz);
extern void *mymalloc(unsigned int size);
