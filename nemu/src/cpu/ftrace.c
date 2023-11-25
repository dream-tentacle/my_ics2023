#include <common.h>
#include <cpu/ftrace.h>
void elf_process(char *file_name) {
  FILE *fp = fopen(file_name, "r");
  Assert(fp, "Can not open '%s'", file_name);
  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, fp) != -1) {
    char *spliter = " ";
    char *token = strtok(line, spliter);
    while (token != NULL) {
      printf("%s\n", token);
      token = strtok(NULL, spliter);
    }
  }
  fclose(fp);
}

// static funct_info *funct_table;