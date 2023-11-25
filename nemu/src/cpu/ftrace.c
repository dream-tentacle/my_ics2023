#include <common.h>
#include <cpu/ftrace.h>
// #include <elf.h>
#include <stdlib.h>

/*
  21: 80000254    32 FUNC    GLOBAL DEFAULT    1 _trm_init
    22: 80009000     0 NOTYPE  GLOBAL DEFAULT    5 _stack_pointer
    23: 80000274     0 NOTYPE  GLOBAL DEFAULT    1 _etext
    24: 80000000     0 NOTYPE  GLOBAL DEFAULT  ABS _pmem_start
    25: 80000294     0 NOTYPE  GLOBAL DEFAULT    5 _bss_start
    26: 80000275     0 NOTYPE  GLOBAL DEFAULT    2 edata
    27: 80000294     4 OBJECT  GLOBAL DEFAULT    5 lvl
    28: 80009000     0 NOTYPE  GLOBAL DEFAULT    5 _heap_start
    29: 80001000     0 NOTYPE  GLOBAL DEFAULT    5 _stack_top
    30: 80000108   168 FUNC    GLOBAL DEFAULT    1 f3
    31: 80009000     0 NOTYPE  GLOBAL DEFAULT    5 end
    32: 800001b0    24 FUNC    GLOBAL DEFAULT    1 check
    33: 80000274     0 NOTYPE  GLOBAL DEFAULT    1 etext
    34: 800000a4   100 FUNC    GLOBAL DEFAULT    1 f2
    35: 80000000     0 FUNC    GLOBAL DEFAULT    1 _start
    36: 00000000     0 NOTYPE  GLOBAL DEFAULT  ABS _entry_offset
    37: 800001c8   128 FUNC    GLOBAL DEFAULT    1 main
    38: 80000010    76 FUNC    GLOBAL DEFAULT    1 f0
    39: 80000275     0 NOTYPE  GLOBAL DEFAULT    2 _data
    40: 80000278    12 OBJECT  GLOBAL DEFAULT    3 ans
    41: 80000284    16 OBJECT  GLOBAL DEFAULT    4 func
    42: 8000005c    72 FUNC    GLOBAL DEFAULT    1 f1
    43: 80009000     0 NOTYPE  GLOBAL DEFAULT    5 _end
    44: 80000248    12 FUNC    GLOBAL DEFAULT    1 halt
    45: 80000298     4 OBJECT  GLOBAL DEFAULT    5 rec
    */
// 只需要FUNC类型的
static funct_info funct_table[10] = {
    {"f0", 0x80000010, 76},    {"f1", 0x8000005c, 72},
    {"f2", 0x800000a4, 100},   {"f3", 0x80000108, 168},
    {"check", 0x800001b0, 24}, {"main", 0x800001c8, 128},
    {"halt", 0x80000248, 12},  {"_trm_init", 0x80000254, 32},
    {"_start", 0x80000000, 0}};

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