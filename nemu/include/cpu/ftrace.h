typedef struct funct_info {
  char *name;
  unsigned int addr;
  unsigned int size;
  struct funct_info *next;
} funct_info;
