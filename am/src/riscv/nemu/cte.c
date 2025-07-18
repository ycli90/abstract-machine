#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 8:  // ecall from MODE_U
      case 11: // ecall from MODE_M
        if (c->GPR1 == -1) ev.event = EVENT_YIELD;
        else ev.event = EVENT_SYSCALL;
        c->mepc += 4;
        break;
      case 0x80000007: // interrupt from timer
        ev.event = EVENT_IRQ_TIMER;
        break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  // initialize mscratch
  asm volatile("csrwi mscratch, 0" : :);

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *c = (Context*)kstack.end - 1;
  c->gpr[2] = (uintptr_t)kstack.end; // $sp
  c->GPR2 = (uintptr_t)arg; // $a0
  c->mepc = (uintptr_t)entry;
  c->mstatus = MODE_M << 11 | 1 << 7;
  c->np = MODE_M;
  c->pdir = NULL;
  return c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
