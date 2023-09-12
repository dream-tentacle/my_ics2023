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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static unsigned int buf_cnt = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"unsigned int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static unsigned int choose(unsigned int n){
  return rand()%n;
}
static void gen(char c){
 	buf[buf_cnt]=c;
	buf_cnt++;
}
static void gen_num(){
	unsigned int add_space = choose(3);
  for(unsigned int i=0;i<add_space;i++)gen(' ');
  unsigned int num = rand();	
	if(num==0){
		gen('0');
		gen('u');	
		add_space = choose(3);
		for(unsigned int i=0;i<add_space;i++)gen(' ');
		return;
	}
	unsigned int exp = 1;
  while(num/exp>=10)exp*=10;
  while(exp>0){
    gen((num/exp) + '0');
    num -= (num/exp)*exp;
    exp/=10; 
  }
	gen('u');
	add_space = choose(3);
  for(unsigned int i=0;i<add_space;i++)gen(' ');
}

static void gen_rand_op(){
  switch(choose(4)){
    case 0: gen('+');break;
		case 1: gen('-');break;
		case 2: gen('*');break;
		default: gen('/');break;
  }
}
static void gen_rand_expr(int leng) {
	if(leng == 0){
		gen_num();
		return;
	}
	else switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(leng-1); gen(')'); break;
    default: gen_rand_expr(leng-1); gen_rand_op(); gen_rand_expr(leng-1); break;
  }
}

int main(int argc, char *argv[]) {
  unsigned int seed = time(0);
  srand(seed);
  unsigned int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  unsigned int i;
  for (i = 0; i < loop; i ++) {
		memset(buf,0,sizeof(buf));	
		buf_cnt=0;
    gen_rand_expr(3);
		sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

		int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
