#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/*
structure of current
typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    _RegSet *tf;
    _Protect as;
    uintptr_t cur_brk;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
  };
} PCB;
*/



/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) { /// didn't understand yet!!!!
  //map memory region [current->max_brk,new_brk] into addr spacce current->as
  //new applied cpace is [max_brk,new_brk)
  if(current->cur_brk==0)//the first
  {
    current->cur_brk=current->max_brk=new_brk;
  }
  else
  {
    if (current->max_brk < new_brk)
    {
      uint32_t first = PGROUNDUP(current->max_brk);
      uint32_t end = PGROUNDDOWN(new_brk);
      if ((new_brk & 0xfff) == 0) 
      {
        end -= PGSIZE;
      }
      for (uint32_t va = first; va <= end; va += PGSIZE) 
      {
        void* pa = new_page();
        _map(&(current->as), (void*)va, pa);
      }
      current->max_brk = new_brk;
    }
    current->max_brk=new_brk;
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);  //use the start of heap as the start address
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
  //initialize the PD and PTs

  //then set CR0 and CR3
  //Format of a Linear Address
/*

linear address's format
 31                 22 21                 12 11                 0
     +---------------------+---------------------+--------------------+
     |                     |                     |                    |
     |         DIR         |        PAGE         |       OFFSET       |
     |                     |                     |                    |
     +---------------------+---------------------+--------------------+


transformation

+-----------+-----------+----------+         +---------------+
              |    DIR    |   PAGE    |  OFFSET  |         |               |
              +-----+-----+-----+-----+-----+----+         |               |
                    |           |           |              |               |
      +-------------+           |           +------------->|    PHYSICAL   |
      |                         |                          |    ADDRESS    |
      |   PAGE DIRECTORY        |      PAGE TABLE          |               |
      |  +---------------+      |   +---------------+      |               |
      |  |               |      |   |               |      +---------------+
      |  |               |      |   |---------------|              ^
      |  |               |      +-->| PG TBL ENTRY  |--------------+
      |  |---------------|          |---------------|
      +->|   DIR ENTRY   |--+       |               |
         |---------------|  |       |               |
         |               |  |       |               |
         +---------------+  |       +---------------+
                 ^          |               ^
+-------+        |          +---------------+
|  CR3  |--------+
+-------+
*/


}
