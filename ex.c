#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* The point of this program is to experiment with what addresses the 
 * system malloc returns.
 */

void *show_malloc(char **ptrs, int i, int size, void *sb) {
        void *next_sb;
        ptrs[i] = malloc(size);
        if(ptrs[i] == NULL) {
                perror("malloc");
                exit(1);
        }
        ptrs[i][0] = 'A';
        next_sb = sbrk(0);
        // print how much memory is allocated, the addres returned by malloc,
        // how much memory much memory is reserved by the program break,
        // the amount of memory reserved by the program break in pages.
        printf("malloc(%d) %p (%p = %ld 4kb pages)\n", size, ptrs[i], 
                        next_sb, (long)(next_sb - sb)/4096);
        return sb;
}

int main() {
	int i = 10;
	while(i!=0){
		int *ad = malloc(sizeof(int)*8);
		i--;
		printf("new %d, %p\n", sizeof(int)*8, ad);	
	}
        char *ptrs[10];

        void *sb = sbrk(0);
        printf("sbrk starts at %p\n", sb);

        // allocate a few small chunks of memory
        // note that addresses re aligned on 32 byte boundaries
        sb = show_malloc(ptrs, 0, 4, sb);
        sb = show_malloc(ptrs, 1, 10, sb);
        sb = show_malloc(ptrs, 2, 4, sb);
        sb = show_malloc(ptrs, 3, 20, sb);

        // allocate a larger chunks (4 kb) of memory
        sb = show_malloc(ptrs, 4, 4096, sb);
        sb = show_malloc(ptrs, 5, 4096, sb);

    // see if freeing a chunk and the allocating another
        // reuses space
        free(ptrs[4]);
        sb = show_malloc(ptrs, 9, 1024, sb);

        // see how much to allocate befor program break moves
        sb = show_malloc(ptrs, 6, 4096*8, sb);
        sb = show_malloc(ptrs, 7, 4096*24, sb);

        sb = show_malloc(ptrs, 8, 4096*32, sb);
        sb = show_malloc(ptrs, 8, 4096*8, sb);
        sb = show_malloc(ptrs, 8, 4096*32, sb);
        sb = show_malloc(ptrs, 8, 4096*32, sb);
        sb = show_malloc(ptrs, 8, 4096*32, sb);

        return 0;
}
