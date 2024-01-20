#include <memory.h>
#include <proc.h>
static void *pf = NULL;
void *new_page(size_t nr_page) {
  pf += nr_page * PGSIZE;
  return pf - nr_page * PGSIZE;
}

#ifdef HAS_VME
static void *pg_alloc(int n) {
  assert(n % PGSIZE == 0);
  void *ret = new_page(n / PGSIZE);
  memset(ret, 0, n);
  return ret;
}
#endif

void free_page(void *p) { panic("not implement yet"); }

extern PCB *current;
/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  if (brk <= current->max_brk) {
    return 0;
  }
  int start = ROUNDUP(current->max_brk, PGSIZE);
  int end = ROUNDUP(brk, PGSIZE);
  for (int i = start; i < end; i += PGSIZE) {
    void *page = new_page(1);
    map(&current->as, (void *)i, page, 0);
    printf("brk %p -> %p\n", i, page);
  }
  current->max_brk = brk;
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  printf("heap.start = %p\n", heap.start);
  printf("heap.end = %p\n", heap.end);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
