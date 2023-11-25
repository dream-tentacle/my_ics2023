#include <common.h>
#include <cpu/ftrace.h>
// #include <elf.h>
#include <stdlib.h>

/*
   10: 80000028     0 NOTYPE  LOCAL  DEFAULT    1 $x
    11: 80000078     0 NOTYPE  LOCAL  DEFAULT    1 $x
    12: 00000000     0 FILE    LOCAL  DEFAULT  ABS trm.c
    13: 8000011c     0 NOTYPE  LOCAL  DEFAULT    1 $x
    14: 80000128     0 NOTYPE  LOCAL  DEFAULT    1 $x
    15: 80000148     1 OBJECT  LOCAL  DEFAULT    2 mainargs
    16: 80000128    32 FUNC    GLOBAL DEFAULT    1 _trm_init
    17: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 _stack_pointer
    18: 80000148     0 NOTYPE  GLOBAL DEFAULT    1 _etext
    19: 80000000     0 NOTYPE  GLOBAL DEFAULT  ABS _pmem_start
    20: 8000019c     0 NOTYPE  GLOBAL DEFAULT    3 _bss_start
    21: 80000149     0 NOTYPE  GLOBAL DEFAULT    2 edata
    22: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 _heap_start
    23: 80001000     0 NOTYPE  GLOBAL DEFAULT    3 _stack_top
    24: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 end
    25: 80000010    24 FUNC    GLOBAL DEFAULT    1 check
    26: 80000148     0 NOTYPE  GLOBAL DEFAULT    1 etext
    27: 80000000     0 FUNC    GLOBAL DEFAULT    1 _start
    28: 00000000     0 NOTYPE  GLOBAL DEFAULT  ABS _entry_offset
    29: 80000078   164 FUNC    GLOBAL DEFAULT    1 main
    30: 80000149     0 NOTYPE  GLOBAL DEFAULT    2 _data
    31: 80009000     0 NOTYPE  GLOBAL DEFAULT    3 _end
    32: 8000011c    12 FUNC    GLOBAL DEFAULT    1 halt
    33: 80000028    80 FUNC    GLOBAL DEFAULT    1 bubble_sort
    */
// 只需要FUNC类型的
// name, addr, size
static funct_info funct_table[10] = {
    {"_trm_init", 0x80000128, 32}, {"check", 0x80000010, 24},
    {"_start", 0x80000000, 0},     {"main", 0x80000078, 164},
    {"halt", 0x8000011c, 12},      {"bubble_sort", 0x80000028, 80}};

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