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
extern void *new_page(size_t nr_page);
uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_ehdr;
  int fd = fs_open(filename, 0, 0);
  if (fd == -1)
    panic("Can not open file %s", filename);
  fs_read(fd, &elf_ehdr, sizeof(Elf_Ehdr));
  // 模数检查
  assert((*(uint32_t *)elf_ehdr.e_ident == 0x464c457f));
  Elf_Phdr elf_phdr[elf_ehdr.e_phnum];
  fs_lseek(fd, elf_ehdr.e_phoff, SEEK_SET);
  fs_read(fd, elf_phdr, sizeof(Elf_Phdr) * elf_ehdr.e_phnum);
  for (int i = 0; i < elf_ehdr.e_phnum; i++) {
    if (elf_phdr[i].p_type == 1) {
      // 从ramdisk中读取数据
      fs_lseek(fd, elf_phdr[i].p_offset, SEEK_SET);
      uint32_t start = ROUNDDOWN(elf_phdr[i].p_vaddr, PGSIZE);
      int j = start;
      for (; j < elf_phdr[i].p_vaddr + elf_phdr[i].p_filesz; j += PGSIZE) {
        void *page = new_page(1);
        map(&pcb->as, (void *)j, page, 0);
        if (j + PGSIZE >= elf_phdr[i].p_vaddr + elf_phdr[i].p_filesz) {
          fs_read(fd, page, elf_phdr[i].p_vaddr + elf_phdr[i].p_filesz - j);
          memset(
              (void *)(page + elf_phdr[i].p_vaddr + elf_phdr[i].p_filesz - j),
              0, PGSIZE - (elf_phdr[i].p_vaddr + elf_phdr[i].p_filesz - j));
        } else {
          fs_read(fd, page, PGSIZE);
        }
      }
      // 未初始化的数据
      for (; j < elf_phdr[i].p_vaddr + elf_phdr[i].p_memsz; j += PGSIZE) {
        void *page = new_page(1);
        map(&pcb->as, (void *)j, page, 0);
        memset(page, 0, PGSIZE);
      }
    }
  }
  Elf64_Shdr elf_shdr[elf_ehdr.e_shnum];
  fs_lseek(fd, elf_ehdr.e_shoff, SEEK_SET);
  fs_read(fd, elf_shdr, sizeof(Elf64_Shdr) * elf_ehdr.e_shnum);
  for (int i = 0; i < elf_ehdr.e_shnum; i++) {
    if (elf_shdr[i].sh_type == 2) {
      // 符号表
      Elf32_Sym elf_sym[elf_shdr[i].sh_size / sizeof(Elf32_Sym)];
      fs_lseek(fd, elf_shdr[i].sh_offset, SEEK_SET);
      fs_read(fd, elf_sym, elf_shdr[i].sh_size);
      for (int j = 0; j < elf_shdr[i].sh_size / sizeof(Elf32_Sym); j++) {
        pcb->max_brk = pcb->max_brk > elf_sym[j].st_value ? pcb->max_brk
                                                          : elf_sym[j].st_value;
      }
      printf("pcb->max_brk = %p\n", pcb->max_brk);
    }
  }
  return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
