#include <common.h>
#include <cpu/ftrace.h>
#include <elf.h>
#include <stdlib.h>

// name, addr, size
static funct_info *funct_table = NULL;
static int func_cnt = 0;
static jmp_log *jmp_head, *jmp_last;
static int funct_layer = 0; // 记录函数嵌套层数
static uint32_t last_pc[1000];
static int last_pc_cnt = 0;
static char *return_name = "ret";
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
  if (!funct_table) {
    FILE *fp =
        fopen("/home/dream/ics2023/navy-apps/apps/pal/build/pal-riscv32", "r");
    if (!fp) {
      printf("open file error\n");
      return;
    }
    Elf32_Ehdr ehdr;
    assert(fread(&ehdr, sizeof(Elf32_Ehdr), 1, fp));
    assert((*(uint32_t *)ehdr.e_ident == 0x464c457f));
    // 读取section header table
    Elf32_Shdr shtab[ehdr.e_shnum];
    printf("ehdr.e_shoff: %x\n", ehdr.e_shoff);
    printf("ehdr.e_shnum: %d\n", ehdr.e_shnum);
    assert(-1 != fseek(fp, ehdr.e_shoff, SEEK_SET));
    assert(fread(shtab, sizeof(Elf32_Sym), ehdr.e_shnum, fp));
    // 寻找.symtab
    int symtab_idx = -1;
    for (int i = 0; i < ehdr.e_shnum; i++) {
      if (shtab[i].sh_type == SHT_SYMTAB) {
        symtab_idx = i;
        break;
      }
    }
    assert(symtab_idx != -1);
    // 读取.symtab
    int symtab_len = shtab[symtab_idx].sh_size / sizeof(Elf32_Sym);
    assert(sizeof(Elf32_Sym) == shtab[symtab_idx].sh_entsize);
    Elf32_Sym symtab[symtab_len];
    for (int i = 0; i < symtab_len; i++) {
      assert(-1 != fseek(fp,
                         shtab[symtab_idx].sh_offset + i * sizeof(Elf32_Sym),
                         SEEK_SET));
      assert(fread(&symtab[i], sizeof(Elf32_Sym), 1, fp));
      if ((symtab[i].st_info & 15) == STT_FUNC) {
        func_cnt++;
      }
    }
    funct_table = malloc(sizeof(funct_info) * func_cnt);
    for (int i = 0; i < symtab_len; i++) {
      if ((symtab[i].st_info & 15) == STT_FUNC) {
        // funct_table[i].name = (char *)(symtab[i].st_name);
        funct_table[i].addr = symtab[i].st_value;
        funct_table[i].size = symtab[i].st_size;
      }
    }
    printf("funct_cnt: %d\n", func_cnt);
  }
  for (int i = 0; i < func_cnt; i++) {
    if (funct_table[i].addr == 0)
      break;
    funct_info *now = &funct_table[i];
    if (now->addr == addr ||
        (now->addr <= addr && now->addr + now->size > addr &&
         (pc < now->addr || pc > now->addr + now->size))) {
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
      printf("│   ");
    }
    if (now->name == return_name) {
      printf("└── %d %s\n", now->layer, now->name);
    } else
      printf("├── %s %d\n", now->name, now->layer);
    now = now->next;
  }
}