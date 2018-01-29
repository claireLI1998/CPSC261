#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

#include "implicit.h"

/*
 * Determine whether or not a block is in use.
 */
static int block_is_in_use(void *block_start)
{
  return 1 & *((block_size_t *) block_start);
}

/*
 * Return the size of a block.
 */
static block_size_t get_block_size(void *block_start)
{
  return -HEADER_SIZE & *((block_size_t *) block_start);
}

/*
 * Return the size of the payload of a block.
 */
static block_size_t get_payload_size(void *block_start)
{
  return get_block_size(block_start) - HEADER_SIZE * 2;
}

/*
 * Find the start of the block, given a pointer to the payload.
 */
static void *get_block_start(void *payload)
{
  return payload - HEADER_SIZE;
}

/*
 * Find the payload, given a pointer to the start of the block.
 */
static void *get_payload(char *block_start)
{
  return block_start + HEADER_SIZE;
}

/*
 * Set the size of a block, and whether or not it is in use. Remember each block
 * has two copies of the header (one at each end).
 */
static void set_block_header(void *block_start, block_size_t block_size, int in_use)
{
  block_size_t header_value = block_size | in_use;
  *((block_size_t *) block_start) = header_value;
  *((block_size_t *) (get_payload(block_start) +
		      get_payload_size(block_start))) = header_value;
}


/*
 * Find the start of the next block.
 */
static void *get_next_block(void *block_start)
{
  return block_start + get_block_size(block_start);
}

/*
 * Find the start of the previous block.
 */
static void *get_previous_block(void *block_start)
{
  /* TO BE COMPLETED BY THE STUDENT. */
  void *starting_point = block_start;
  starting_point = starting_point - HEADER_SIZE;
  block_size_t previous_block_size = *((block_size_t *) starting_point);
  starting_point = starting_point - previous_block_size + HEADER_SIZE;

  return starting_point;
}

/*
 * Determine whether or not the given block is at the front of the heap.
 */
static int is_first_block(heap *h, void *block_start)
{
  return block_start == h->start;
}

/*
 * Determine whether or not the given address is inside the heap
 * region. Can be used to loop through all blocks:
 *
 * for (blk = h->start; is_within_heap_range(h, blk); blk = get_next_block(blk)) ...
 */
static int is_within_heap_range(heap *h, void *addr)
{
  return addr >= h->start && addr < h->start + h->size;

}

/*
 * Coalesce a block with its consecutive block, only if both blocks are free.
 * Return a pointer to the beginning of the coalesced block.
 */
static void *coalesce(heap *h, void *first_block_start)
{
  /* TO BE COMPLETED BY THE STUDENT. */

  void *save = first_block_start;
  block_size_t one = 0;
  int count;
  //int use1 = is_whithin_heap_range(h,next_block_start);
  //int use2 = is_whithin_heap_range(h,first_block_start);
  for(count = 2; is_within_heap_range(h,first_block_start); first_block_start = get_next_block(first_block_start)){
    if(!block_is_in_use(first_block_start)){
      one += get_block_size(first_block_start);
      count --;
  }
    if(count == 0){
      break;
    }
  }
  set_block_header(save, one, 0);
  return first_block_start;
}


/*
 * Determine the size of the block we need to allocate given the size
 * the user requested. Don't forget we need space for the header  and
 * footer.
 */
static block_size_t get_size_to_allocate(block_size_t user_size)
{
  /* TO BE COMPLETED BY THE STUDENT. */
  //payload_align_t available_size = 8;
  //while(user_size > available_size){
  //  available_size *2 ;
  //}

  return user_size + 2 * HEADER_SIZE;
}

/*
 * Turn a free block into one the user can utilize. Split the block if
 * it's more than twice as large or MAX_UNUSED_BYTES bytes larger than
 * needed.
 */
static void *prepare_block_for_use(void *block_start, block_size_t real_size)
{
  /* TO BE COMPLETED BY THE STUDENT. */

    block_size_t block_size1 = get_block_size(block_start);
    void *next_block1 = get_next_block(block_start);
    if(block_size1 > 2 * (real_size) || block_size1 > (real_size + MAX_UNUSED_BYTES)){
        set_block_header(block_start, real_size, 1);
        set_block_header(next_block1, block_size1-real_size, 0);
    }
    else{
      set_block_header(block_start, block_size1, 0);
    }
   return block_start + HEADER_SIZE;
}

/*
 * Create a heap that is "size" bytes large.
 */
heap *heap_create(intptr_t size, search_alg_t search_alg)
{
  /* Allocate space in the process' actual heap */
  void *heap_start = sbrk(size);
  if (heap_start == (void *) -1) return NULL;

  /* Use the first part of the allocated space for the heap header */
  heap *h = heap_start;
  heap_start += sizeof(heap);
  size -= sizeof(heap);

  /* Ensures the heap_start points to an address that has space for
     the header, while allowing the payload to be aligned to PAYLOAD_ALIGN */
  int delta = PAYLOAD_ALIGN - HEADER_SIZE - ((uintptr_t) heap_start) % PAYLOAD_ALIGN;
  if (delta < 0)

    delta += PAYLOAD_ALIGN;
  if (delta > 0) {
    heap_start += delta;
    size -= delta;
  }
  /* Ensures the size points to as many bytes as necessary so that
     only full-sized blocks fit into the heap.
   */
  size -= (size - 2 * HEADER_SIZE) % PAYLOAD_ALIGN;

  h->size = size;
  h->start = heap_start;
  h->search_alg = search_alg;

  h->next = h->start;
  set_block_header(h->start, size, 0);
  return h;
}
/*
 * Print the structure of the heap to the screen.
 */
void heap_print(heap *h)
{
  /* TO BE COMPLETED BY THE STUDENT. */
  void *block;
  for (block = h->start; is_within_heap_range(h, block); block = get_next_block(block)){
    printf("Block at address %p\n", block);
    block_size_t t1 = get_block_size(block);
    int use = block_is_in_use(block);
    printf(" Size: %d\n", t1);
    if(use == 1){
     printf(" In use: Yes\n");
     printf("\n\n");
    }
    else{
    printf(" In use: NO\n");
    printf("\n\n");
   }
  }
}

/*
 * Determine the average size of a free block.
 */
block_size_t heap_find_avg_free_block_size(heap *h)
{
  /* TO BE COMPLETED BY THE STUDENT. */
  void *block;
  int sum = 0;
  block_size_t size = 0;
  block_size_t avg_size;
  for (block = h->start; is_within_heap_range(h, block); block = get_next_block(block)){
    int use = block_is_in_use(block);
    if(use == 0){
      size += get_block_size(block);
      ++sum;
    }

  }
  avg_size = size / sum;
  return avg_size;
}

/*
 * Free a block on the heap h. Beware of the case where the  heap uses
 * a next fit search strategy, and h->next is pointing to a block that
 * is to be coalesced.
 */
void heap_free(heap *h, void *payload)
{
  /* TO BE COMPLETED BY THE STUDENT. */
  void *current_block = get_block_start(payload);
  void *previous_block = get_previous_block(current_block);
  void *next_block = get_previous_block(current_block);

  block_size_t current_size = 0;
  //int previous_size;
  //int next_size;

  int usage1 = block_is_in_use(next_block);
  int usage2 = block_is_in_use(previous_block);

  if(usage1 == 1){
    current_block = coalesce(h, previous_block);
    current_size += get_block_size(previous_block);
  }
  if(usage2 == 1){
    current_block = coalesce(h, previous_block);
    current_size += get_block_size(previous_block);
  }
  set_block_header(current_block, current_size, 0);

  h -> start = current_block;

  free(previous_block);
  free(next_block);
}

/*
 * Malloc a block on the heap h, using first fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static void *malloc_first_fit(heap *h, block_size_t user_size)
{
  /* TO BE COMPLETED BY THE STUDENT. */

  void *current_addr_ff = h -> start;
  int inuse = 0;
  for(current_addr_ff = h -> start; is_within_heap_range(h, current_addr_ff);current_addr_ff = get_next_block(current_addr_ff)){
     inuse = block_is_in_use(current_addr_ff);
     if((inuse == 1) || (get_block_size(current_addr_ff) < (user_size + HEADER_SIZE * 2))){

     }
     else{
       return prepare_block_for_use(current_addr_ff, user_size);
       break;
     }
 }

 return NULL;

}

/*
 * Malloc a block on the heap h, using best fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static void *malloc_best_fit(heap *h, block_size_t user_size)
{
  /* TO BE COMPLETED BY THE STUDENT. */
  block_size_t wasted_space = 2147483647;
  void *current_addr_bf = h -> start;
  void *best_fit_addr = NULL;
  for (current_addr_bf = h->start; is_within_heap_range(h, current_addr_bf); current_addr_bf = get_next_block(current_addr_bf)){
    if( (!block_is_in_use(current_addr_bf)) && ((get_block_size(current_addr_bf)) >= (user_size + HEADER_SIZE * 2))){
      block_size_t unused_space = get_block_size(current_addr_bf) - user_size - HEADER_SIZE *2;
      if(unused_space < wasted_space){
        best_fit_addr = current_addr_bf;
        wasted_space = unused_space;
      }
    }
  }
  if(current_addr_bf == NULL){
    return NULL;
  }
  else
    return prepare_block_for_use(best_fit_addr, user_size);


}

/*
 * Malloc a block on the heap h, using next fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static void *malloc_next_fit(heap *h, block_size_t user_size)
{
  /* TO BE COMPLETED BY THE STUDENT. */
  void *current_addr_nf = h -> next;

  int inuse = block_is_in_use(current_addr_nf);
  while( inuse == 1 || (get_block_size(current_addr_nf) < user_size + HEADER_SIZE *2)){
    if(!is_within_heap_range(h, current_addr_nf)){
      return NULL;
    }
    else{
      current_addr_nf = get_next_block(current_addr_nf);
    }
  }
    h -> next = current_addr_nf;

   return prepare_block_for_use(current_addr_nf, user_size);
}

/*
 * Our implementation of malloc.
 */
void *heap_malloc(heap *h, block_size_t size)
{
  switch (h->search_alg) {
  case HEAP_FIRSTFIT:
    return malloc_first_fit(h, size);
  case HEAP_NEXTFIT:
    return malloc_next_fit(h, size);
  case HEAP_BESTFIT:
    return malloc_best_fit(h, size);
  }
  return NULL;
}
