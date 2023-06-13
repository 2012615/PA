#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  //push EFFLAGS CS EIP one by one, 
  memcpy(&t1,&cpu.eflags,sizeof(cpu.eflags));
  rtl_li(&t0,t1);
  rtl_push(&t0);
  rtl_push(&cpu.cs);
  rtl_li(&t0,ret_addr);
  rtl_push(&t0);
  //save the environment

  vaddr_t enter=cpu.idtr.base+NO*sizeof(GateDesc);
  assert(enter<=cpu.idtr.base+cpu.idtr.limit);

  uint32_t offsetLow=vaddr_read(enter,2);
  uint32_t offsetHIgh=vaddr_read(enter+sizeof(GateDesc)-2,2);
  uint32_t target=(offsetHIgh<<16)+offsetLow;
#ifdef DEBUG
  Log("target_addr=0x%x", target);
#endif
  decoding.is_jmp=1;
  decoding.jmp_eip=target;
//  TODO();
}

void dev_raise_intr() {
}

