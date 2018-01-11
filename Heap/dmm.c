#include <stdio.h>  // needed for size_t
#include <unistd.h> // needed for sbrk
#include <assert.h> // needed for asserts
#include "dmm.h"

typedef struct metadata {
    /* size_t is the return type of the sizeof operator. Since the size of an
     * object depends on the architecture and its implementation, size_t is used
     * to represent the maximum size of any object in the particular
     * implementation. size contains the size of the data object or the number of
     * free bytes
     */
    size_t size;
    struct metadata* next;
    struct metadata* prev;
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */
static metadata_t* freelist = NULL;

void print_list(void* target) {
    metadata_t* freelist_head = (metadata_t*) target;
    while(freelist_head != NULL) {
        printf("\nTarget Size:%zd, Head:%p, Prev:%p, Next:%p\n",
        freelist_head->size,
        freelist_head,
        freelist_head->prev,
        freelist_head->next);
        freelist_head = freelist_head->next;
    }
}

static metadata_t* findFit(size_t size) {
    metadata_t* freelist_head = freelist;
    while((freelist_head->size > 0) && (freelist_head->next != NULL)) {
        if(freelist_head->size >= size) {
            return freelist_head;
        }
        freelist_head = freelist_head->next;
    }
    return NULL;
}

void place(metadata_t* target, size_t size) {
    size_t remainder = target->size - size;
    /* split */
    if(remainder >= METADATA_T_ALIGNED + ALIGN(1)) {
        metadata_t* new_block = ((metadata_t*) (target + 1));
        void* tmp = ((void*) new_block) + size;
        new_block = (metadata_t*) tmp;
        target->prev->next = new_block;
        target->next->prev = new_block;

        new_block->prev = target->prev;
        new_block->next = target->next;
        new_block->size = remainder - METADATA_T_ALIGNED;
	
	if(freelist == target) {
            freelist = new_block;
        }
        target->prev = NULL;
        target->next = NULL;
        target->size = size;
    } else {
        if(freelist == target) {
            freelist = target->next;
        }
        target->prev->next = target->next;
        target->next->prev = target->prev;
        target->prev = NULL;
        target->next = NULL;
    }
}

void* dmalloc(size_t numbytes) {
    if(numbytes == 0) return NULL;

    /* initialize through sbrk call first time */
    if(freelist == NULL) {      
        if(!dmalloc_init())
            return NULL;
    }

    /* search and place */
    size_t alignedSize = ALIGN(numbytes);
    metadata_t* fit = findFit(alignedSize);
    printf("Called!\n");

    if(fit != NULL) {
        place(fit, alignedSize);
        fit = (metadata_t*) (fit + 1);
        //print_list(freelist->prev);
        return fit;
    }
    printf("Called null!\n");
    return NULL;
}

void coalesce(metadata_t* head_ptr, metadata_t* prev_ptr, metadata_t* next_ptr) {
    void* tmp = ((void*) prev_ptr) + prev_ptr->size + METADATA_T_ALIGNED;
    bool prev_alloc = ((metadata_t*) tmp) != head_ptr;
    tmp = ((void*) head_ptr) + head_ptr->size + METADATA_T_ALIGNED;
    bool next_alloc = ((metadata_t*) tmp) != next_ptr;

    if(prev_ptr->size == 0) {
        prev_alloc = true;
        freelist = head_ptr;
    }
    if(next_ptr->size == 0) {
        next_alloc = true;
    }

    if(prev_alloc && next_alloc) {
        head_ptr->prev = prev_ptr;
        head_ptr->next = next_ptr;
        prev_ptr->next = head_ptr;
        next_ptr->prev = head_ptr;
    } else if(prev_alloc && !next_alloc) {
        head_ptr->prev = prev_ptr;
        head_ptr->next = next_ptr->next;
        head_ptr->size += next_ptr->size + METADATA_T_ALIGNED;
        prev_ptr->next = head_ptr;
        head_ptr->next->prev = head_ptr;
    } else if(!prev_alloc && next_alloc) {
        prev_ptr->size += head_ptr->size + METADATA_T_ALIGNED;
    } else {
        prev_ptr->size += head_ptr->size + next_ptr->size + 2*METADATA_T_ALIGNED;
        prev_ptr->next = next_ptr->next;
        next_ptr->next->prev = prev_ptr;
    }
}

void dfree(void* ptr) {
    metadata_t* prev_ptr = freelist->prev;
    metadata_t* next_ptr = freelist;
    metadata_t* head_ptr = (metadata_t*) (ptr-METADATA_T_ALIGNED);
    while(next_ptr != NULL) {
        if(prev_ptr < head_ptr && head_ptr < next_ptr) {
            coalesce(head_ptr, prev_ptr, next_ptr);
            break;
        }
        prev_ptr = prev_ptr->next;
        next_ptr = next_ptr->next;
    }
    //print_list(freelist->prev);
}

bool dmalloc_init() {
    size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
    /* returns heap_region, which is initialized to freelist */
    metadata_t* epilogue = (metadata_t*) sbrk(max_bytes);
    /* Q: Why casting is used? i.e., why (void*)-1? */
    if(epilogue == (void*) - 1) {
        return false;
    }

    /* create prologue and shift position in memory of freelist */
    epilogue->prev = NULL;
    epilogue->next = NULL;
    epilogue->size = max_bytes - METADATA_T_ALIGNED;

    metadata_t* prologue = epilogue;
    epilogue = (metadata_t*) (epilogue + 1);
    prologue->prev = NULL;
    prologue->next = epilogue;
    epilogue->prev = prologue;
    epilogue->size = max_bytes - 2*METADATA_T_ALIGNED;
    prologue->size = 0;

    freelist = epilogue;
    void* tmp = ((void*) epilogue) + epilogue->size;
    epilogue = (metadata_t*) tmp;
    freelist->prev = prologue;
    freelist->next = epilogue;
    epilogue->prev = freelist;
    freelist->size = max_bytes - 3*METADATA_T_ALIGNED;
    epilogue->size = 0;

    return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
    metadata_t* freelist_head = freelist;
    while(freelist_head != NULL) {
        DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
        freelist_head->size,
        freelist_head,
        freelist_head->prev,
        freelist_head->next);
        freelist_head = freelist_head->next;
    }
    DEBUG("\n");
}
