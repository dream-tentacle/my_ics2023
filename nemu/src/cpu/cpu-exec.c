/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of
 *the Mulan PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
 *OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

#include "../monitor/sdb/sdb.h"
#ifdef CONFIG_DEVICE
#include <device/map.h>
#endif

#ifdef FTRACE
#include <cpu/ftrace.h>
#endif
/* The assembly code of instructions executed is only output to the
 * screen when the number of instructions executed is less than this
 * value. This is useful when you use the `si' command. You can modify
 * this value as you want.
 */
#define MAX_INST_TO_PRINT 10
void init_regex();
void init_wp_pool();

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

char ring_buffer[21][100];
int ring_cnt;
Decode *last_decode;

void device_update();

#ifdef TRACE_DEVICE
void print_device_buffer() {
  for (int i = 1; i <= device_buffer_cnt; i++) {
    printf("%s\n", device_buffer[i]);
  }
}
#endif
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) {
    log_write("%s\n", _this->logbuf);
  }
#endif
  if (g_print_step) {
    IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
  }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
  WP *now = get_head();
  while (now != NULL) {
    bool flag = true;
    bool *p = &flag;
    word_t result = expr(now->expr, p);
    if (flag) {
      if (result != now->last_result) {
        now->last_result = result;
        nemu_state.state = NEMU_STOP;
        printf("A watchpoint is triggered:\n");
        printf("%s = %u = 0x%x\n", now->expr, result, result);
      }
    } else {
      printf("Invalid watchpoint expression: \"%s\"\n", now->expr);
    }
    now = now->next;
  }
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i--) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0)
    space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
              MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc),
              (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
  if (ring_cnt == 20) {
    for (int i = 1; i <= 19; i++) {
      strcpy(ring_buffer[i], ring_buffer[i + 1]);
    }
    strcpy(ring_buffer[20], s->logbuf);
  } else {
    strcpy(ring_buffer[++ring_cnt], s->logbuf);
  }
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (; n > 0; n--) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst++;
    if (nemu_state.state != NEMU_RUNNING)
      break;
    IFDEF(CONFIG_DEVICE, device_update());
    trace_and_difftest(&s, cpu.pc);
#ifdef TRACE_DEVICE
    if (nemu_state.state != NEMU_RUNNING) {
      print_device_buffer();
      break;
    }
#endif
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0)
    Log("simulation frequency = " NUMBERIC_FMT " inst/s",
        g_nr_guest_inst * 1000000 / g_timer);
  else
    Log("Finish running in less than 1 us and can not calculate the "
        "simulation frequency");
}
#ifdef CONFIG_ITRACE
void print_ring_buffer() {
  for (int i = 1; i <= ring_cnt; i++) {
    printf("     %s\n", ring_buffer[i]);
  }
  char *p = last_decode->logbuf;
  p += snprintf(p, sizeof(last_decode->logbuf), FMT_WORD ":", last_decode->pc);
  int ilen = last_decode->snpc - last_decode->pc;
  int i;
  uint8_t *inst = (uint8_t *)&last_decode->isa.inst.val;
  for (i = ilen - 1; i >= 0; i--) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0)
    space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, last_decode->logbuf + sizeof(last_decode->logbuf) - p,
              MUXDEF(CONFIG_ISA_x86, last_decode->snpc, last_decode->pc),
              (uint8_t *)&last_decode->isa.inst.val, ilen);
#else
  p[0] = '\0';
#endif
  printf(" --> %s\n", last_decode->logbuf);
}
#endif
void assert_fail_msg() {
  isa_reg_display();
  statistic();
  printf("123");
#ifdef CONFIG_ITRACE
  print_ring_buffer();
#endif
#ifdef TRACE_DEVICE
  print_device_buffer();
#endif
#ifdef FTRACE
  printf("ftrace:\n");
  print_jmp_log();
#endif
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
  case NEMU_END:
  case NEMU_ABORT:
    printf("Program execution has ended. To restart the program, "
           "exit NEMU "
           "and run again.\n");
    return;
  default:
    nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
  case NEMU_RUNNING:
    nemu_state.state = NEMU_STOP;
    break;

  case NEMU_END:
  case NEMU_ABORT:

#ifdef FTRACE
    print_jmp_log();
#endif
    Log("nemu: %s at pc = " FMT_WORD,
        (nemu_state.state == NEMU_ABORT
             ? ANSI_FMT("ABORT", ANSI_FG_RED)
             : (nemu_state.halt_ret == 0
                    ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN)
                    : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
        nemu_state.halt_pc);
    // fall through
  case NEMU_QUIT:
    statistic();
  }
}
