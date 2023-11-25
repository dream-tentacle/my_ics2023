typedef struct funct_info {
  char *name;
  unsigned int addr;
  unsigned int size;
} funct_info;
// void elf_process(char *file_name);

typedef struct jmp_log {
  char *name;
  unsigned int layer;
  struct jmp_log *next;
} jmp_log;

void call_funct(unsigned int addr);
void print_jmp_log();
