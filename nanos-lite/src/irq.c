#include "common.h"
//sent events according to their type
extern _RegSet* do_syscall(_RegSet* r);
extern _RegSet* schedule(_RegSet* prev);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:  //value is 8. Defined in (nexus-am/am/am.h)
      do_syscall(r);
      return schedule(r);  //pa4
    case _EVENT_TRAP:
      //printf("SELF-TRAPPED\n");
      return schedule(r);//switch the process, and return the new process's context
    case _EVENT_IRQ_TIME:
      Log("IRQ_TIME\n");
      return schedule(r);
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
