#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
  //_switch(&pcb[i].as);
  //current = &pcb[i];
  //((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

_RegSet* schedule(_RegSet *prev) {
  //save the context pointer
  if(current!=NULL)
  {
    current->tf=prev;
  }

  //when we execute 2 processes, the xianjian will be very slow.
  //so we need to set a 'level', can be replaced by the time that process
  //has been executed, the xianjian will get more chances.
  static int num=0;
  static const int frequency=1000;

  if(current==&pcb[0])
  {
    num++;
  }  
  else
  {
    current=&pcb[0];//xianjian 
  }


  if(num==frequency)
  {
    current=&pcb[1];
    num=0;
  }


  //select pcb[0] as the new process
  //current =(current==&pcb[0]? &pcb[1] : &pcb[0]);

  //Log("ptr=0x%x\n",(uint32_t)current->as.ptr);

  _switch(&current->as); //switch the addr space
  return current->tf; //return the new process's context
}
