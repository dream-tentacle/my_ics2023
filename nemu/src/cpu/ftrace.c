#include <common.h>
#include <cpu/ftrace.h>
#include <elf.h>
void elf_process(char *file_name) {
  FILE *fp = fopen(file_name, "r");
  Assert(fp, "Can not open '%s'", file_name);

  fclose(fp);
}

// static funct_info *funct_table;