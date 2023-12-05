#include <proc.h>
#include <elf.h>
#include <ramdisk.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  // assert(*(uint32_t *)elf->e_ident == 0x464c457f);
  ramdisk_read((void *)0x83000000, 0, 0x04d4c);
  ramdisk_read((void *)0x83005000, 0x005000, 0x00898);
  memset((void *)(0x83005000 + 0x00898), 0, 0x008d4);
  return 0x830000b4;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
