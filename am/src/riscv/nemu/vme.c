#include <am.h>
#include <nemu.h>
#include <klib.h>
#include <riscv/riscv.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    assert(((intptr_t)segments[i].start & (PGSIZE - 1)) == 0);
    assert(((intptr_t)segments[i].end & (PGSIZE - 1)) == 0);
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, MMAP_READ | MMAP_WRITE);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  assert(((uintptr_t)va & (PGSIZE - 1)) == 0);
  assert(((uintptr_t)pa & (PGSIZE - 1)) == 0);
  uintptr_t page_dir_addr = (uint32_t)as->ptr;
  uint32_t* page_dir_entry = (uint32_t*)page_dir_addr + ((uint32_t)va >> 22);
  uintptr_t page_table_addr;
  if ((*page_dir_entry & PTE_V) == 0) {
    page_table_addr = (uintptr_t)pgalloc_usr(PGSIZE);
    uint32_t property = PTE_V;
    if (prot & MMAP_USER) property |= PTE_U;
    *page_dir_entry = page_table_addr >> 12 << 10 | property;
  } else {
    page_table_addr = *page_dir_entry >> 10 << 12;
  }
  uint32_t* page_table_entry = ((uint32_t*)page_table_addr) + (((uint32_t)va >> 12) & ((1 << 10) - 1));
  if ((*page_table_entry & PTE_V) == PTE_V) {
    printf("vaddr %p remapped to %p\n", va, pa);
    assert(0);
  }
  uint32_t property = PTE_V;
  if (prot & MMAP_READ) property |= PTE_R | PTE_X;
  if (prot & MMAP_WRITE) property |= PTE_W;
  if (prot & MMAP_USER) property |= PTE_U;
  *page_table_entry = (uint32_t)pa >> 12 << 10 | property;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = (Context*)kstack.end - 1;
  c->mepc = (uintptr_t)entry;
  c->mstatus = MODE_U << 11 | 0x3 << 18 | 1 << 7;
  c->np = MODE_U;
  if (vme_enable) c->pdir = as->ptr;
  return c;
}
