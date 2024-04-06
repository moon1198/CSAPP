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
#include <stdbool.h>
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
static int classify(size_t size);
static void place(void *bp, size_t asize);
static void delete(void *bp);



static char *heap_listp;
static void **class_list;

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define CLASS_SIZE 20


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


//不同类的指针之间；无法赋值和比较，所以需要多次类型转换
//#define PRE_CHAIN(bp) (((unsigned int *) bp))
//#define NEX_CHAIN(bp) (((unsigned int *) bp + 1))


#define PRE_CHAIN(bp) ((void **) bp)
#define NEX_CHAIN(bp) ((void **) bp + 1)

//Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))



#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - ini
 * tialize the malloc package.
 */
int mm_init(void)
{
	if((heap_listp = mem_sbrk(4 * (WSIZE + CLASS_SIZE))) == (void*) -1){
		return -1;
	}
	class_list = (void *) heap_listp;
	for (int i=0; i<20; i++){
		class_list[i] = NULL;
	}
	heap_listp += (4 * CLASS_SIZE);
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


static int classify(size_t size){
	int class;
	for (class = 0; class < 20; class ++){
		if (size <= (1 << (class + 4))){
			return class;
		}
	}
	//the variable class equals 20 after loop
	//the error occur in here
	class = 19;
	return class;
}


//this two approach are available,the second is faster 
//#define PUT_PTR(p , val)	(*(unsigned int *)(p) = (unsigned int)(val))
//将一个指针的值赋予给一个指针
#define PUT_PTR(p , val)	(*(void **)(p) = (void *)(val))


static void insert(void *bp){
	void *temp;
	int bk_size = GET_SIZE(HDRP(bp));
	int class = classify(bk_size);
	temp = class_list[class];	//保留下一个节点
	
	class_list[class] = bp;	//链接跟

	PUT_PTR(PRE_CHAIN(bp), &class_list[class]);		//链接子节点
	PUT_PTR(NEX_CHAIN(bp), temp);
	if(temp != NULL){
		PUT_PTR(PRE_CHAIN(temp), bp);
	}
}

static void delete(void *bp){
	if ((void *)(*NEX_CHAIN(bp)) != NULL){	
		if ((void *)*PRE_CHAIN(bp) < (void *)(&class_list[20])){
		//是头不是尾
		PUT_PTR(*PRE_CHAIN(bp), *NEX_CHAIN(bp));
		PUT_PTR(PRE_CHAIN(*NEX_CHAIN(bp)), *PRE_CHAIN(bp));

		}else{
		//在中间
		PUT_PTR(NEX_CHAIN(*PRE_CHAIN(bp)), *NEX_CHAIN(bp));
		PUT_PTR(PRE_CHAIN(*NEX_CHAIN(bp)), *PRE_CHAIN(bp));
		}
	}else if ((void *)*PRE_CHAIN(bp) < (void *)(&class_list[20])){
		PUT_PTR(*PRE_CHAIN(bp), NULL);	//是尾又是头
	}else {	//是尾不是头
		PUT_PTR(NEX_CHAIN(*PRE_CHAIN(bp)), NULL);
	}
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
	int class = classify(asize);
	bool flag = 1;
	for(;class < 20;class++){
		if (class_list[class] != NULL){
			flag = 0;
		}
		for(bp = class_list[class]; !flag; bp = ((void *) *NEX_CHAIN(bp))){
			if (((void *) *NEX_CHAIN(bp)) == NULL){
				flag = 1;
			}
			if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= asize)){
				return bp;
			}
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
		insert(NEXT_BLKP(bp));
	}else{
		PUT(HDRP(bp), PACK(bsize, 1));
		PUT(FTRP(bp), PACK(bsize, 1));
	}
	delete(bp);
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
	size_t prev_size, next_size;
	next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
	prev_size = GET_SIZE(HDRP(PREV_BLKP(bp)));
	//state 0:0
	if(!prev && !next){
		delete(NEXT_BLKP(bp));
		delete(PREV_BLKP(bp));
		asize = GET_SIZE(HDRP(bp)) + prev_size  + next_size;
		PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 0));
		bp = PREV_BLKP(bp);
		PUT(HDRP(bp), PACK(asize, 0));
	}
	//state 0:1
	else if(!prev && next){
		asize = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(PREV_BLKP(bp)));
		delete(PREV_BLKP(bp));
		PUT(FTRP(bp), PACK(asize, 0));
		bp = PREV_BLKP(bp);
		PUT(HDRP(bp), PACK(asize, 0));
	}
	//state 1:0
	else if(prev && !next){
		asize = GET_SIZE(HDRP(bp)) +  GET_SIZE(HDRP(NEXT_BLKP(bp)));
		delete(NEXT_BLKP(bp));
		PUT(HDRP(bp), PACK(asize, 0));
		PUT(FTRP(bp), PACK(asize, 0));
	}
	//state 1:1
	else if(prev && next){
		asize = GET_SIZE(HDRP(bp));
		PUT(HDRP(bp), PACK(asize, 0));
		PUT(FTRP(bp), PACK(asize, 0));
	}
	insert(bp);

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




