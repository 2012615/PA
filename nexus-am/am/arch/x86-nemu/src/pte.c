#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  //p->ptr can get the base addr of PD
  PDE* pgdir=(PDE*)p->ptr;
  PTE* pgtab=NULL; 

  PDE* pd=pgdir+PDX(va);
  if(!(*pd & PTE_P))//not avaliable
  {
    //set a new pt
    pgtab=(PTE*)(palloc_f());
    *pd=(uintptr_t)pgtab|PTE_P;
  }
  pgtab=(PTE*)PTE_ADDR(*pd);//the base addr

  PTE* pt=pgtab+PTX(va);
  //set the value of pt index
  *pt=(uintptr_t)pa|PTE_P;
}

void _unmap(_Protect *p, void *va) {
}
/*
structure of PCB

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    _RegSet *tf;  trap frame
    _Protect as;
    uintptr_t cur_brk;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
  };
} PCB;
*/
//create a scene for user porcess
//initialize a tf whose return addr is entry; set stack frame of _strat(navy-apps/libs/libc/src/start.c)
/*
| |
+---------------+ <---- ustack.end
| stack frame |
| of _start() |
+---------------+
| |
| trap frame |
| |
+---------------+ <--+
| | |
| | |
| | |
| | |
+---------------+ |
| tf | ---+
+---------------+ <---- ustack.start
| |
*/
//_start accept 3 params

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  extern void* memcpy(void*, const void*, int);
  int arg1=0;
  char* arg2=NULL;

  memcpy((void*)ustack.end - 4,  (void*)arg2, 4);
  memcpy((void*)ustack.end - 8,  (void*)arg2, 4);
  memcpy((void*)ustack.end - 12, (void*)arg1, 4);
  memcpy((void*)ustack.end - 16, (void*)arg1, 4); //eip of _start

  //trap frame initialize cs to 8,eflags=2, return eip is entry
  _RegSet tf;
  tf.cs=8;
  tf.eip=(uintptr_t)entry;
  tf.eflags=0x02|FL_IF;
  
  //the addr that saves tf
  void* ptf = (void*)(ustack.end - 16 - sizeof(_RegSet));
  memcpy(ptf, (void*)&tf, sizeof(_RegSet));

  return (_RegSet*)ptf;
}
