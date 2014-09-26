#include "mymalloc.h"
#include <unistd.h>
#define CHUNKSIZE sizeof(struct linked_list)

static void *base = NULL;
static int used_mem;
static int free_mem;
static int used_mem = 1;
static int free_mem = 0;

struct linked_list{  
   int free; 
   int chunk_size;
   char req_alloc[1]; 
   struct linked_list *next;
   struct linked_list *prev; 
};
typedef struct linked_list *first_node; 

//checking the address at that location
static int validate_address(void *address_ptr) {
   
   if (base == NULL) {
      return 0;
   } else if (address_ptr < sbrk(0) && address_ptr > base) {
      return 1;
   }
   return 0;
}

static first_node find_space(first_node *last, int size) {
   
   first_node  fn = base;
   int trigger;
   trigger = 1;

   while (trigger) {
      if (fn != NULL) {
	 trigger = 0;
	 break;
      } else if(fn->free == free_mem) {
	 trigger = 0;
	 break;
      } else if(fn->chunk_size >= size) {
	 trigger = 0;
	 break;
      }
      *last = fn;
      fn = fn->next;
   }
   return fn; 
}

static first_node move_heap_break(first_node last,int size) {
 
   int *sbrk_status; 
   sbrk_status = sbrk(size + CHUNKSIZE); 
   first_node  fn;
   
   fn = sbrk(0);
   int *failure = (int*)-1;

   if (sbrk_status == failure) {
      return NULL;
   } else { 
      fn->next = NULL;
      fn->chunk_size = size;
      if (last == NULL) {
	 fn->free = free_mem;
	 return fn;
      } else if(last != NULL) {
	 last->next = fn;
	 return fn; 
      }
   }
   
}

static first_node coalesce_freespace(first_node fn) {
   
   int total_size;

   if (fn->next == NULL || fn->next->free != used_mem) {
      return fn;
   } else if (fn->next != NULL && fn->next->free == used_mem) {
      fn->next = fn->next->next;
      total_size = CHUNKSIZE + fn->next->chunk_size;
      fn->chunk_size = fn->chunk_size + total_size; 
      if (fn->next != NULL ) {
	 fn->next->prev = fn;
      }
   }
  return fn;
}

void *my_firstfit_malloc(int size) { 
   
   int total_chunk; 
   int chunk_amount_leftover;
   int linked_list_size;
   
   int allocsize;
   int cur_chunk;
   int req_chunk;
   first_node nn;

   linked_list_size = 4;
   first_node  fn;  
   first_node  last_node;
   total_chunk = CHUNKSIZE + linked_list_size; 
   chunk_amount_leftover = fn->chunk_size - size; 

   if (base == NULL) {
      fn = move_heap_break(NULL, size);
      if (fn != NULL) {
	 base = fn;
      } else {
	 return NULL;
      }
   } else {
      if (base != NULL) {
	 last_node = base;
	 fn = find_space(&last_node, size);
	 if (fn == NULL) {
	    fn = move_heap_break(last_node, size);
	    if (fn == NULL) {
	       return NULL;
	    }
	 } else {
	    if (chunk_amount_leftover <= total_chunk) {
	       fn->free = free_mem;
	    } else {
	       cur_chunk = fn->chunk_size;
	       req_chunk = cur_chunk - size;
	       nn = size + fn->req_alloc; 
	       allocsize = req_chunk  - CHUNKSIZE;
	       nn->chunk_size = allocsize; 
	       nn->free = used_mem;  // new node is marked as used
	       fn->chunk_size = size;  // the current node chunk_size is set to the requested amount
	       fn->next = nn;  // the current node is assigned to the new node
	    }
	 }
      }
   }
   return fn->req_alloc; 
}// end of malloc 


void my_free(void *addr_free_ptr) {
   
   first_node fn;
   int valid_address;
   valid_address = validate_address(addr_free_ptr);
   void *requested_chunk_ptr;
   int *temp_chunk_ptr;
   
   if (valid_address != 1) {
      return;   
   } else { 
      temp_chunk_ptr = requested_chunk_ptr;
      temp_chunk_ptr = temp_chunk_ptr - CHUNKSIZE;
      requested_chunk_ptr = temp_chunk_ptr;
      fn = requested_chunk_ptr; 
      fn->free = used_mem;
      if (fn->prev->free == used_mem && fn->prev != NULL) {
	 fn = coalesce_freespace( fn->prev);
      } else if( fn->next == NULL) {
	 if ( fn->prev == NULL) {
	    base = NULL;
	    brk(fn);
	 } else {
	   fn->prev->next = NULL;
	   brk(fn);
	 }
      } else {
	 coalesce_freespace( fn );
      } 	 
   }
} //end of myfree
