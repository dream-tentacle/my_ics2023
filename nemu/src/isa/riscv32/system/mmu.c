/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>
extern word_t satp;
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  printf("vaddr = %x\n", vaddr);
  int pdir = ((satp & 0x3fffff) << 12); // 页目录基地址
  printf("pdir: %x\n", pdir);
  long long pde_p = pdir | (vaddr >> 22 << 2);
  printf("pde_p: %llx\n", pde_p);
  uint32_t pde = paddr_read((paddr_t)pde_p, 4);
  printf("pde: %x\n", pde);
  assert((pde & 1));                       // 页目录项有效
  uint32_t ptab = (pde << 2) & 0xfffff000; // 页表基地址
  printf("ptab: %x\n", ptab);
  uint32_t pte = paddr_read(ptab, (vaddr >> 12) & 0x3ff);
  printf("pte: %x\n", pte);
  assert((pte & 1)); // 页表项有效
  printf("%x\n", (pte >> 10 << 12) | (vaddr & 0xfff));
  // 暂时设定页表项的物理地址等于虚拟地址
  assert(((pte >> 10 << 12) | (vaddr & 0xfff)) == vaddr);
  return (pte >> 10 << 12) | (vaddr & 0xfff); // 物理地址
}
