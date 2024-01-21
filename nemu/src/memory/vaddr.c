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
#include <memory/paddr.h>
extern paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type);
extern word_t satp;
word_t vaddr_ifetch(vaddr_t addr, int len) {
  if (isa_mmu_check(addr, len, MMU_FETCH) == MMU_DIRECT)
    return paddr_read(addr, len);
  // if ((satp << 12) == 0x821dd000)
  //   printf("ifetch vaddr %x\n", addr);
  addr = isa_mmu_translate(addr, len, MEM_TYPE_IFETCH);
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  if (isa_mmu_check(addr, len, MMU_LOAD) == MMU_DIRECT)
    return paddr_read(addr, len);
  // if (addr >> 4 == 0x40060a3 || addr >> 4 == 0x82248a3) {
  //   printf("addr=%x,", addr);
  // }
  // if ((satp << 12) == 0x821dd000)
  //   printf("read vaddr %x, ", addr);
  addr = isa_mmu_translate(addr, len, MEM_TYPE_READ);
  // if ((satp << 12) == 0x821dd000)
  //   printf("to %x\n", addr);
  // if (addr >> 4 == 0x40060a3 || addr >> 4 == 0x82248a3) {
  //   printf("previous=%x\n", paddr_read(addr, len));
  // }
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  // if (addr >> 4 == 0x40060a3 || addr >> 4 == 0x82248a3) {
  //   printf("addr=%x,data=%x,previous=%x\n", addr, data, vaddr_read(addr,
  //   len));
  // }
  if (isa_mmu_check(addr, len, MMU_STORE) == MMU_DIRECT)
    paddr_write(addr, len, data);
  else {
    // if ((satp << 12) == 0x821dd000)
    //   printf("write vaddr %x, ", addr);
    addr = isa_mmu_translate(addr, len, MEM_TYPE_WRITE);
    // if ((satp << 12) == 0x821dd000)
    //   printf("to %x\n", addr);
    paddr_write(addr, len, data);
  }
}
