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
  int pdir = ((satp & 0x3fffff) << 12); // 页目录基地址
  long long x = pdir | (vaddr >> 22 << 2);
  printf("pdir | (vaddr >> 22 << 2) = %llx\n", x);
  uint32_t *pde_p = (uint32_t *)x; // 页目录项
  printf("pde_p = %p\n", pde_p);
  uint32_t pde = *pde_p;
  printf("pde = %x\n", pde);
  assert(pde & 1); // 页目录项有效
  uint32_t *ptab = (uint32_t *)(long long)(pde >> 10 << 12); // 页表基地址
  uint32_t pte = ptab[(vaddr >> 12) & 0x3ff];                // 页表项
  assert((pte & 1));                                         // 页表项有效
  // 暂时设定页表项的物理地址等于虚拟地址
  assert(((pte >> 10 << 12) | (vaddr & 0xfff)) == vaddr);
  return (pte >> 10 << 12) | (vaddr & 0xfff); // 物理地址
}
