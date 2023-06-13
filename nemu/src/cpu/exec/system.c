#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  //set the start address and length
  //TODO();
  t1=id_dest->val;
  rtl_lm(&t0,&t1,2);
  cpu.idtr.limit=t0;

  t1=id_dest->val+2;
  rtl_lm(&t0,&t1,4);
  cpu.idtr.base=t0;
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

extern void raise_intr(uint8_t NO,vaddr_t ret_addr);

make_EHelper(int) {  //call an exception
  //TODO();
  uint8_t TrapNo=id_dest->val &0xff;
  raise_intr(TrapNo,decoding.seq_eip); //save the addr to return

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}


make_EHelper(iret) {  //the exception has been processed, go back to the former ststus
  //TODO();
  rtl_pop(&cpu.eip); //pop the reg and save the value in the param
  rtl_pop(&cpu.cs);
  rtl_pop(&t0);
  memcpy(&cpu.eflags,&t0,sizeof(cpu.eflags));

  decoding.jmp_eip = 1;
  decoding.seq_eip = cpu.eip;
  print_asm("iret");
}

/*
uint32_t pio_read(ioaddr_t addr, int len) {
  assert(len == 1 || len == 2 || len == 4);
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  pio_callback(addr, len, false);		// prepare data to read, not to write
  uint32_t data = *(uint32_t *)(pio_space + addr) & (~0u >> ((4 - len) << 3));
  return data;
}

void pio_write(ioaddr_t addr, int len, uint32_t data) {
  assert(len == 1 || len == 2 || len == 4);
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  memcpy(pio_space + addr, &data, len);
  pio_callback(addr, len, true);
}
*/




uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);
//to input or output something?
//I/O operation
make_EHelper(in) {
  //TODO();
  //pio_read->param is the dest address and width of data
  rtl_li(&t0,pio_read(id_src->val,id_dest->width));
  operand_write(id_dest,&t0);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //TODO();

  pio_write(id_dest->val,id_src->width,id_src->val);


  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

