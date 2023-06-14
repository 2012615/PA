#----|-------entry-------|-errorcode-|---irq id---|---handler---|
.globl vecsys;    vecsys:  pushl $0;  pushl $0x80; jmp asm_trap
.globl vecnull;  vecnull:  pushl $0;  pushl   $-1; jmp asm_trap
#push the errorcode and irq(0x81)  then junp to asm_trap
.globl vecself;  vecself:  pushl $0;  pushl $0x81; jmp asm_trap  #added in pa4
.globl vectime;  vectime:  pushl $0;  pushl $32;   jmp asm_trap
asm_trap:
  pushal

  pushl %esp
  call irq_handle

  #addl $4, %esp
  movl %eax,%esp  #switch the stack top

  popal
  addl $8, %esp

  iret
