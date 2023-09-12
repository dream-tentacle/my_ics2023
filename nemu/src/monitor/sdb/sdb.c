/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state=NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  if (args==NULL)cpu_exec(1);
  else cpu_exec(atoi(args));
  return 0;
}

static int cmd_info(char *args){
  if (args[0]=='r'){
    isa_reg_display();
  }
  else if (args[0]=='w'){
    /* TODO */
    
  }
  else{
    printf("%s: wrong argumant",args);
  }
  return 0;
}

static int cmd_x(char *args){
  char *arg = strtok(args," ");
  unsigned int print_len = atoi(arg);
  arg = strtok(NULL," ");
  unsigned int start_pos = strtol(arg, NULL, 16);
  printf("start at: %u\n",start_pos);
  for(unsigned int i=start_pos;i<=print_len*4+start_pos;i+=4){
    printf("0x%08x\n",paddr_read(i,4));
  }
  return 0;
}

static int cmd_calcu(char *args){
	bool flag=true;
	bool *p = &flag;
	word_t result = expr(args,p);
	if(flag){
		printf("%u\n",result);
	}
	return 0;
}

static int cmd_test_calcu(char *args){
	char file_path[100];
	if(args==NULL)strcpy(file_path,"/home/dreamtouch/ics2023/nemu/tools/gen-expr/input");
	else strcpy(file_path,args);
	FILE *fp = NULL;
	char *buf=NULL, *ans=NULL;
	fp = fopen(file_path,"r");
	size_t len = 0;
	int cor=0;
	while(getline(&buf,&len,fp)!=-1){
		ans=strtok(buf," ");
		buf=strtok(NULL," ");
		bool flag=true;
		bool *p=&flag;
		word_t result = expr(buf,p);
		if(flag && result==atoi(ans))cor++;
	}
	fclose(fp);
	printf("%d\n",cor);
	return 0;
}	


static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si","Format: 'si N'. Step N instuctions. If N is not provided, it will be set to 1", cmd_si},
  {"info","Format: 'info N'. Print information. N must be r or w",cmd_info},
  {"x","Format: 'x N EXPR", cmd_x},
	{"calcu","Calculate the expression", cmd_calcu},
	{"test_calcu","Test the function of calcu",cmd_test_calcu}
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
