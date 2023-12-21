#include <common.h>
#include <cpu/ftrace.h>
#include <elf.h>
#include <stdlib.h>

// name, addr, size
static funct_info funct_table[1273];
static int func_cnt = 0;
static jmp_log jmp_head[1000];
static int funct_layer = 0; // 记录函数嵌套层数
static uint32_t last_pc[1000];
static int last_pc_cnt = 0;
static char *return_name = "ret";
void call_funct(unsigned int addr, unsigned int pc) {
  if (last_pc_cnt > 0 && last_pc[last_pc_cnt - 1] == addr - 4) {
    last_pc_cnt--;
    funct_layer--;
    return;
  }
  if (!func_cnt) {
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
    assert(-1 != fseek(fp, ehdr.e_shoff, SEEK_SET));
    assert(fread(shtab, sizeof(Elf32_Shdr), ehdr.e_shnum, fp));
    // 寻找.symtab和.strtab
    int symtab_idx = -1;
    int strtab_idx = ehdr.e_shstrndx;
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
    Elf32_Sym symtab;
    for (int i = 0; i < symtab_len; i++) {
      assert(-1 != fseek(fp,
                         shtab[symtab_idx].sh_offset + i * sizeof(Elf32_Sym),
                         SEEK_SET));
      assert(fread(&symtab, sizeof(Elf32_Sym), 1, fp));
      if ((symtab.st_info & 15) == STT_FUNC) {
        int offset = shtab[strtab_idx].sh_offset + symtab.st_name;
        char *name = (char *)malloc(100);
        assert(-1 != fseek(fp, offset, SEEK_SET));
        assert(fread(name, 100, 1, fp));
        funct_table[func_cnt].name = name;
        funct_table[func_cnt].addr = symtab.st_value;
        funct_table[func_cnt].size = symtab.st_size;
        func_cnt++;
      }
    }
    printf("funct_cnt: %d\n", func_cnt);
  }
  for (int i = 0; i < func_cnt; i++) {
    funct_info *now = &funct_table[i];
    if (now->addr == addr ||
        (now->addr <= addr && now->addr + now->size > addr &&
         (pc < now->addr || pc > now->addr + now->size))) {
      // printf("now->addr: %x, addr: %x\n", now->addr, addr);
      // if (now->addr <= addr && now->addr + now->size > addr) {
      last_pc[last_pc_cnt++] = pc;
      jmp_head[funct_layer].name = now->name;
      jmp_head[funct_layer].layer = funct_layer;
      jmp_head[funct_layer].now_pc = pc;
      funct_layer++;
      printf("%s %d\n", now->name, funct_layer);
      return;
    }
  }
  // printf("pc: %x\n", pc);
}

void print_jmp_log() {
  for (int i = 0; i < funct_layer; i++) {
    jmp_log *now = &jmp_head[i];
    printf("%x ", now->now_pc);
    for (int i = 1; i < now->layer; i++) {
      printf("│   ");
    }
    if (now->name == return_name) {
      printf("└── %d %s\n", now->layer, now->name);
    } else
      printf("├── %s %d\n", now->name, now->layer);
  }
}