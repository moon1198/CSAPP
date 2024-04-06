/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
void *best_fit(size_t asize);



static char *heap_listp;

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)


/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define PUT(p, val)	 (*(unsigned int *)(p) = (val))
#define GET(p)	(*(unsigned int *)(p))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp) ((char *)(bp) - (WSIZE))
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))



//Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))



#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - ini
 * tialize the malloc package.
 */
int mm_init(void)
{
	if((heap_listp = mem_sbrk(4 * WSIZE)) == (void*) -1){
		return -1;
	}
	PUT(heap_listp, 0);
	PUT(heap_listp + 1*WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 2*WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 3*WSIZE, PACK(0, 1));

	heap_listp += (4*WSIZE);

	if (extend_heap(CHUNKSIZE/WSIZE) == NULL){
		return -1;
	}
    return 0;
}

static void *extend_heap(size_t words){

	char* bp;
	size_t size;
	size = ALIGN(words* WSIZE);			//byte 
	if((bp = mem_sbrk(size)) == (void*) -1){
		return NULL;
	}
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
	
	return coalesce(bp);	

}



/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
/*	
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
*/	

	
	size_t extendsize;
    size_t newsize;
	void *bp;

	if (size ==  0){
		return NULL;
	}
	else if (size < DSIZE){
		newsize = 2 * DSIZE;
	}
	else {
		newsize = ((size + (DSIZE - 1) + DSIZE) / DSIZE) * DSIZE;
	}

	//search the fit address to place
	if((bp = find_fit(newsize)) != NULL){
		place(bp, newsize);	
		return bp;
	}

	//not found,should extend the heap
	extendsize = MAX(newsize, CHUNKSIZE);
	extendsize /= WSIZE;
	if ((bp = extend_heap(extendsize)) == NULL){
		return NULL;
	}
	place(bp, newsize);
	return bp;
	
}
static void *find_fit(size_t asize){
	void *bp;
	
	for(bp = heap_listp; GET_SIZE(HDRP(bp)) != 0; bp = NEXT_BLKP(bp)){
		if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= asize)){
			return bp;
		}
	}
	return NULL;
}

static void place(void *bp, size_t asize){
	size_t bsize;
	if(((bsize = GET_SIZE(HDRP(bp))) - asize) >= (2 * DSIZE)){
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		PUT(HDRP(NEXT_BLKP(bp)), PACK(bsize - asize, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(bsize - asize, 0));
	}else{
		PUT(HDRP(bp), PACK(bsize, 1));
		PUT(FTRP(bp), PACK(bsize, 1));
	}
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	void *bp;
	bp = ptr;
	PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 0));
	PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 0));
	coalesce(bp);
}
static void *coalesce(void *bp){
	int prev = GET_ALLOC(HDRP(PREV_BLKP(bp)));
	int next = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t asize;

	//state 0:0
	if(!prev && !next){
		asize = GET_SIZE(HDRP(bp)) +  GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 0));
		bp = PREV_BLKP(bp);
		PUT(HDRP(bp), PACK(asize, 0));
	}
	//state 0:1
	else if(!prev && next){
		asize = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(asize, 0));
		bp = PREV_BLKP(bp);
		PUT(HDRP(bp), PACK(asize, 0));
	}
	//state 1:0
	else if(prev && !next){
		asize = GET_SIZE(HDRP(bp)) +  GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(asize, 0));
		PUT(FTRP(bp), PACK(asize, 0));
	}
	//state 1:1
	else if(prev && next){
		asize = GET_SIZE(HDRP(bp));
		PUT(HDRP(bp), PACK(asize, 0));
		PUT(FTRP(bp), PACK(asize, 0));
	}
	return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

void *mm_realloc(void *ptr, size_t size)
{

	void *oldptr = ptr;
	void *newptr;
	void *bp;
	size_t copySize;
	if((oldptr != NULL) && (size != 0)){
		newptr = mm_malloc(size);
		if (newptr == NULL){
			return NULL;
		}
		copySize = GET_SIZE(HDRP(oldptr));
		if (copySize > size){
			copySize = size;
		}
		memcpy(newptr, oldptr, copySize);
		mm_free(oldptr);
	}else if(oldptr == NULL){
		bp = mm_malloc(size);
		return bp;
	}else{
		mm_free(oldptr);
		return NULL;
	}
	return newptr;
}




/*

void *mm_realloc(void *ptr, size_t size)
{
	    void *oldptr = ptr;
		    void *newptr;
			    size_t copySize;

				    newptr = mm_malloc(size);
					    if (newptr == NULL)
							      return NULL;
						    size = GET_SIZE(HDRP(oldptr));
							    copySize = GET_SIZE(HDRP(newptr));
								    if (size < copySize)
										      copySize = size;
									    memcpy(newptr, oldptr, copySize-WSIZE);
										    mm_free(oldptr);
											    return newptr;
}

*/




