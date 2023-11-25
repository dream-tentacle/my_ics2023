#include <common.h>
#include <cpu/ftrace.h>
// #include <elf.h>
#include <stdlib.h>
// void elf_process(char *file_name) {
//   FILE *fp = fopen(file_name, "r");
//   Assert(fp, "Can not open '%s'", file_name);

//   fclose(fp);
// }

static funct_info funct_table[10] = {{"_trm_init", 0x80000108, 32},
                                     {"check", 0x80000010, 24},
                                     {"_start", 0x80000000, 0},
                                     {"main", 0x80000028, 212},
                                     {"halt", 0x800000fc, 12}};

static jmp_log *jmp_head, *jmp_last;
static int funct_layer = 0;  // 记录函数嵌套层数
static uint32_t last_pc[1000];
static int last_pc_cnt = 0;
static char *return_name = "return";
void call_funct(unsigned int addr, unsigned int pc) {
  if (last_pc_cnt > 0 && last_pc[last_pc_cnt - 1] == pc) {
    last_pc_cnt--;
    funct_layer--;
    if (jmp_last == NULL) {
      jmp_last = malloc(sizeof(jmp_log));
      jmp_head = jmp_last;
      jmp_head->name = return_name;
      jmp_head->layer = funct_layer;
      jmp_head->next = NULL;
    } else {
      jmp_last->next = malloc(sizeof(jmp_log));
      jmp_last = jmp_last->next;
      jmp_last->name = return_name;
      jmp_last->layer = funct_layer;
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
        jmp_head->next = NULL;
      } else {
        jmp_last->next = malloc(sizeof(jmp_log));
        jmp_last = jmp_last->next;
        jmp_last->name = now->name;
        jmp_last->layer = funct_layer;
        jmp_last->next = NULL;
      }
      return;
    }
  }
}

void print_jmp_log() {
  jmp_log *now = jmp_head;
  while (now != NULL) {
    for (int i = 1; i < now->layer; i++) {
      printf(" %c", i == now->layer - 1 ? ' ' : '|');
    }
    printf("%s %d\n", now->name, now->layer);
    now = now->next;
  }
}