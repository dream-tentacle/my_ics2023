#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void *(*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void *) = NULL;
static int vme_enable = 0;

static Area segments[] = { // Kernel memory mappings
    NEMU_PADDR_SPACE};

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

bool vme_init(void *(*pgalloc_f)(int), void (*pgfree_f)(void *)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
  printf("kernel page table base = %p\n", kas.ptr);

  int i;
  for (i = 0; i < LENGTH(segments); i++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }
  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE *)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}
// va
// |VPN[1]|VPN[0]|Offset|
// |  10  |  10  |  12  |
// pa
// |PPN[1]|PPN[0]|Offset|
// |  10  |  10  |  12  |
// PTE
// |PPN[1]|PPN[0]| ... | V |
// |  12  |  10  |    10   |
#define VPN1(va) (((uintptr_t)(va) >> 22) & 0x3ff)
#define VPN0(va) (((uintptr_t)(va) >> 12) & 0x3ff)
#define OFFSET(va) ((uintptr_t)(va) & 0xfff)
#define PPN1(pa) (((uintptr_t)(pa) >> 22) & 0x3ff)
#define PPN0(pa) (((uintptr_t)(pa) >> 12) & 0x3ff)
#define PTE_PPN1(pte) (((uintptr_t)(pte) >> 20) & 0x3ff)
#define PTE_PPN0(pte) (((uintptr_t)(pte) >> 10) & 0x3ff)
#define PTE_PPN(pte) (((uintptr_t)(pte) >> 10) & 0x3fffff)
char flag = 0;
void map(AddrSpace *as, void *va, void *pa, int prot) {
  // 首先获取页目录项
  PTE *page_dir = as->ptr;
  PTE *page_dir_entry_p = &page_dir[VPN1(va)];
  if ((int)pa == 0x821e7000) {
    printf("va = %p, pa = %p, as->ptr = %p\n", va, pa, as->ptr);
    printf("page_dir_entry_p = %p, *page_dir_entry_p = %p\n", page_dir_entry_p,
           *page_dir_entry_p);
  }
  // 如果页目录项不存在，那么就分配一个页表
  if (!(*page_dir_entry_p & 1)) {
    PTE *page_table = (PTE *)(pgalloc_usr(PGSIZE));
    memset(page_table, 0, PGSIZE);
    *page_dir_entry_p = ((uintptr_t)page_table >> 2) | 0x1;
    if ((int)page_table == 0x821c7000) {
      printf("va = %p, pa = %p, as->ptr = %p\n", va, pa, as->ptr);
      printf("VPN1(va) = %d, VPN0(va) = %d, OFFSET(va) = %d\n", VPN1(va),
             VPN0(va), OFFSET(va));
    }
  }
  // 获取页表项
  PTE *page_table = (PTE *)(PTE_PPN(*page_dir_entry_p) << 12);
  PTE *page_table_entry_p = &page_table[VPN0(va)];
  if (as->ptr != kas.ptr)
    printf("va = %p, pa = %p, as->ptr = %p\n", va, pa, as->ptr);

  // 检查是不是没有填入过，若填入过则报错
  if ((*page_table_entry_p & 1)) {
    printf("has mapped\n");
    printf("va = %p, pa = %p, as->ptr = %p\n", va, pa, as->ptr);
    printf("VPN1(va) = %d, VPN0(va) = %d, OFFSET(va) = %d\n", VPN1(va),
           VPN0(va), OFFSET(va));
    printf("page_dir + 4 * VPN1(va) = %p\n", (int)page_dir + 4 * VPN1(va));
    printf("page_dir_entry_p = %p, *page_dir_entry_p = %p\n", page_dir_entry_p,
           *page_dir_entry_p);
    printf("page_table_entry_p = %p, *page_table_entry_p = %p\n",
           page_table_entry_p, *page_table_entry_p);
    assert(0);
  }
  // 填入页表项
  *page_table_entry_p = ((uintptr_t)pa >> 2) | 0x1;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = (Context *)(kstack.end - sizeof(Context));
  c->pdir = as->ptr;
  c->mepc = (uint32_t)entry;
  return c;
}
