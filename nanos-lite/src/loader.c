#include <proc.h>
#include <elf.h>
#include <ramdisk.h>
#include "fs.h"

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_ehdr;
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, &elf_ehdr, sizeof(Elf_Ehdr));
  // 模数检查
  assert((*(uint32_t *)elf_ehdr.e_ident == 0x464c457f));
  Elf_Phdr elf_phdr[elf_ehdr.e_phnum];
  printf("elf_ehdr.e_phnum = %d\n", elf_ehdr.e_phnum);
  for (int i = 0; i < elf_ehdr.e_phnum; i++) {
    printf("Type = %d\n", elf_phdr[i].p_type);
    printf("Offset = %d\n", elf_phdr[i].p_offset);
    printf("Vaddr = %d\n", elf_phdr[i].p_vaddr);
    printf("Physaddr = %d\n", elf_phdr[i].p_paddr);
    printf("Filesz = %d\n", elf_phdr[i].p_filesz);
    printf("Memsz = %d\n", elf_phdr[i].p_memsz);
    printf("Flags = %d\n", elf_phdr[i].p_flags);
    if (elf_phdr[i].p_type == 1) {
      // 从ramdisk中读取数据
      fs_lseek(fd, elf_phdr[i].p_offset, SEEK_SET);
      fs_read(fd, (void *)elf_phdr[i].p_vaddr, elf_phdr[i].p_memsz);
      // 将未初始化的数据置为0
      memset((void *)(elf_phdr[i].p_vaddr + elf_phdr[i].p_filesz), 0,
             elf_phdr[i].p_memsz - elf_phdr[i].p_filesz);
    }
  }
  return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
