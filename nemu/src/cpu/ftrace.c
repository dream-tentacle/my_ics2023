#include <common.h>
#include <cpu/ftrace.h>
// #include <elf.h>
#include <stdlib.h>

/*
    17: 800003b4    32 FUNC    GLOBAL DEFAULT    1 _trm_init
    18: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 _stack_pointer
    19: 800003d4     0 NOTYPE  GLOBAL DEFAULT    1 _etext
    20: 80000000     0 NOTYPE  GLOBAL DEFAULT  ABS _pmem_start
    21: 80000428     0 NOTYPE  GLOBAL DEFAULT    3 _bss_start
    22: 800003d5     0 NOTYPE  GLOBAL DEFAULT    2 edata
    23: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 _heap_start
    24: 80001000     0 NOTYPE  GLOBAL DEFAULT    3 _stack_top
    25: 80000028   200 FUNC    GLOBAL DEFAULT    1 partition
    26: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 end
    27: 80000010    24 FUNC    GLOBAL DEFAULT    1 check
    28: 800003d4     0 NOTYPE  GLOBAL DEFAULT    1 etext
    29: 80000000     0 FUNC    GLOBAL DEFAULT    1 _start
    30: 00000000     0 NOTYPE  GLOBAL DEFAULT  ABS _entry_offset
    31: 800002e8   192 FUNC    GLOBAL DEFAULT    1 main
    32: 800003d5     0 NOTYPE  GLOBAL DEFAULT    2 _data
    33: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 _end
    34: 800003a8    12 FUNC    GLOBAL DEFAULT    1 halt
    35: 800003d8    80 OBJECT  GLOBAL DEFAULT    3 a
    36: 800000f0   504 FUNC    GLOBAL DEFAULT    1 quick_sort
    */
// 只需要FUNC类型的
// name, addr, size
static funct_info funct_table[10] = {
    {"_trm_init", 0x800003b4, 32},  {"partition", 0x80000028, 200},
    {"check", 0x80000010, 24},      {"_start", 0x80000000, 0},
    {"main", 0x800002e8, 192},      {"halt", 0x800003a8, 12},
    {"quick_sort", 0x800000f0, 504}};

static jmp_log *jmp_head, *jmp_last;
static int funct_layer = 0;  // 记录函数嵌套层数
static uint32_t last_pc[1000];
static int last_pc_cnt = 0;
static char *return_name = "return";
void call_funct(unsigned int addr, unsigned int pc) {
  if (last_pc_cnt > 0 && last_pc[last_pc_cnt - 1] == addr - 4) {
    last_pc_cnt--;
    funct_layer--;
    if (jmp_last == NULL) {
      jmp_last = malloc(sizeof(jmp_log));
      jmp_head = jmp_last;
      jmp_head->name = return_name;
      jmp_head->layer = funct_layer + 1;
      jmp_head->now_pc = pc;
      jmp_head->next = NULL;
    } else {
      jmp_last->next = malloc(sizeof(jmp_log));
      jmp_last = jmp_last->next;
      jmp_last->name = return_name;
      jmp_last->layer = funct_layer + 1;
      jmp_last->next = NULL;
    }
    return;
  }
  for (int i = 0; i < 10; i++) {
    if (funct_table[i].addr == 0) break;
    funct_info *now = &funct_table[i];
    if (now->addr == addr) {
      funct_layer++;
      last_pc[last_pc_cnt++] = pc;
      if (jmp_last == NULL) {
        jmp_last = malloc(sizeof(jmp_log));
        jmp_head = jmp_last;
        jmp_head->name = now->name;
        jmp_head->layer = funct_layer;
        jmp_head->now_pc = pc;
        jmp_head->next = NULL;
      } else {
        jmp_last->next = malloc(sizeof(jmp_log));
        jmp_last = jmp_last->next;
        jmp_last->name = now->name;
        jmp_last->layer = funct_layer;
        jmp_last->now_pc = pc;
        jmp_last->next = NULL;
      }
      return;
    }
  }
}

void print_jmp_log() {
  jmp_log *now = jmp_head;
  while (now != NULL) {
    printf("%x ", now->now_pc);
    for (int i = 1; i < now->layer; i++) {
      printf("|   ");
    }
    printf("%s %d\n", now->name, now->layer);
    now = now->next;
  }
}