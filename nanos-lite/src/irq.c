#include "common.h"
//sent events according to their type
extern _RegSet* do_syscall(_RegSet* r);
extern _RegSet* schedule_(_RegSet* prev);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:  //value is 8. Defined in (nexus-am/am/am.h)
      return do_syscall(r);
      //return schedule_(r);
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
