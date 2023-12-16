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
  Elf_Ehdr elf_ehdr;
  ramdisk_read(&elf_ehdr, 0, sizeof(Elf_Ehdr));
  // 模数检查
  assert((*(uint32_t *)elf_ehdr.e_ident == 0x464c457f));
  Elf_Phdr elf_phdr;
  for (int i = 0; i < elf_ehdr.e_phnum; i++) {
    ramdisk_read(&elf_phdr, elf_ehdr.e_phoff + sizeof(Elf_Phdr) * i,
                 sizeof(Elf_Phdr));
    if (elf_phdr.p_type == 1) {
      // 从ramdisk中读取数据
      ramdisk_read((void *)elf_phdr.p_vaddr, elf_phdr.p_offset,
                   elf_phdr.p_memsz);
      // 将未初始化的数据置为0
      memset((void *)(elf_phdr.p_vaddr + elf_phdr.p_filesz), 0,
             elf_phdr.p_memsz - elf_phdr.p_filesz);
    }
  }
  return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
