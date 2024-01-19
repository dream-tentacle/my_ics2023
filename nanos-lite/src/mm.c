#include <memory.h>

static void *pf = NULL;
#define PAGE_SIZE (4 << 10)
void *new_page(size_t nr_page) {
  if (pf == NULL)
    pf = heap.end;
  pf -= nr_page * PAGE_SIZE;
  return pf + nr_page * PAGE_SIZE; // 返回的是页的最高地址
}

#ifdef HAS_VME
static void *pg_alloc(int n) { return NULL; }
#endif

void free_page(void *p) { panic("not implement yet"); }

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) { return 0; }

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
