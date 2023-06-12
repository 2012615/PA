#include "cpu/exec.h"

/*
TEST (Test) performs the logical "and" of the two operands, 
clears OF and CF, leaves AF undefined, and updates SF, ZF, and PF. 
The flags can be tested by conditional control transfer instructions 
or by the byte-set-on-condition instructions. The operands may be 
doublewords, words, or bytes
*/

make_EHelper(test) {
  //TODO();
  rtl_and(&t2,&id_dest->val,&id_src->val);

  rtl_update_ZFSF(&t2,id_dest->width);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  
  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2,id_dest->width); 

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);


  print_asm_template2(and);
}

make_EHelper(xor) {

  rtl_xor(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  /*
  AND, OR, and XOR clear OF and CF, 
   leave AF undefined, and update SF, ZF, and PF.
  */

  rtl_update_ZFSF(&t2,id_dest->width); //This part not sure!!!

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);


  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();

  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(or);
}

make_EHelper(sar) {  //right shift, and plus 0 if pos, 1 if neg
  //TODO();
  // unnecessary to update CF and OF in NEMU

  rtl_sext(&t2,&id_dest->val,id_dest->width); // t2=id_dest
  rtl_sar(&t2,&t2,&id_src->val);

  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU

  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();
  rtl_not(&id_dest->val);
  operand_write(id_dest,&id_dest->val);
  print_asm_template1(not);
}

make_EHelper(rol)
{
  rtl_shri(&t2, &id_dest->val, id_dest->width * 8 - id_src->val);
  rtl_shl(&t3, &id_dest->val, &id_src->val);
  rtl_or(&t1, &t2, &t3);
  operand_write(id_dest, &t1);

  print_asm_template2(rol);
}

