#include <proc.h>
#include <elf.h>
#include <ramdisk.h>

uintptr_t loader(PCB *pcb, const char *filename);

void naive_uload(PCB *pcb, const char *filename);
