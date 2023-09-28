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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_NUM = 1,
  TK_HEX_NUM = 2,
  TK_DEREF = 3,
  TK_REG = 4,
  TK_EQ = 5,
  TK_NEQ = 6,
  TK_AND = 7,
  TK_MINUS = 8,
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", '+'},      // plus
    {"==", TK_EQ},     // equal
    {"0x[0-9A-F]+u?", TK_HEX_NUM},
    {"[0-9]+u?", TK_NUM},
    {"\\$[\\$a-z0-9]+", TK_REG},
    {"\\-", '-'},
    {"\\(", '('},
    {"\\)", ')'},
    {"\\*", '*'},
    {"\\/", '/'},
    {"!=", TK_NEQ},
    {"&&", TK_AND},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  printf("%s\n", e);
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
        //     rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_NOTYPE:
          break;
        case TK_NUM:
          tokens[nr_token].type = TK_NUM;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          nr_token++;
          break;
        case TK_HEX_NUM:
          tokens[nr_token].type = TK_HEX_NUM;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          nr_token++;
          break;
        case TK_REG:
          tokens[nr_token].type = TK_REG;
          strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
          nr_token++;
          break;
        default:
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

word_t eval(int p, int q, bool *success);
word_t expr(char *e, bool *success) {
  memset(tokens, 0, sizeof(tokens));
  nr_token = 0;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' &&
        (i == 0 || tokens[i - 1].type == '(' || tokens[i - 1].type == '+' ||
         tokens[i - 1].type == '-' || tokens[i - 1].type == '*' ||
         tokens[i - 1].type == '/' || tokens[i - 1].type == TK_EQ ||
         tokens[i - 1].type == TK_NEQ || tokens[i - 1].type == TK_AND)) {
      tokens[i].type = TK_DEREF; // 解引用
    }
    if (tokens[i].type == '-' &&
        (i == 0 || tokens[i - 1].type == '(' || tokens[i - 1].type == '+' ||
         tokens[i - 1].type == '-' || tokens[i - 1].type == '*' ||
         tokens[i - 1].type == '/' || tokens[i - 1].type == TK_EQ ||
         tokens[i - 1].type == TK_NEQ || tokens[i - 1].type == TK_AND)) {
      tokens[i].type = TK_MINUS; // 负数
    }
  }
  word_t result = eval(0, nr_token - 1, success);
  return result;
}
bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')')
    return false;
  int parenthese = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(')
      parenthese++;
    else if (tokens[i].type == ')')
      parenthese--;
    if (parenthese == 0 && i != q)
      return false;
  }
  return parenthese == 0;
}
word_t eval(int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    return 0;
  } else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_NUM) {
      word_t result = atoi(tokens[p].str);
      *success = true;
      return result;
    }
    if (tokens[p].type == TK_HEX_NUM) {
      char *endptr;
      word_t result = strtol(tokens[p].str, &endptr, 16);
      *success = true;
      return result;
    }
    if (tokens[p].type == TK_REG) {
      word_t result = isa_reg_str2val(tokens[p].str, success);
      return result;
    }
    assert(0);
  } else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  } else {
    int parenthese = 0;
    int op = -1;
    int op_type = -1;
#define find(tok)                                                              \
  {                                                                            \
    parenthese = 0;                                                            \
    for (int i = q; i >= p && op == -1; i--) {                                 \
      if (tokens[i].type == '(')                                               \
        parenthese--;                                                          \
      else if (tokens[i].type == ')')                                          \
        parenthese++;                                                          \
      else if (parenthese == 0) {                                              \
        if (tokens[i].type == tok) {                                           \
          op_type = tokens[i].type;                                            \
          op = i;                                                              \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }
#define find2(tok1, tok2)                                                      \
  {                                                                            \
    parenthese = 0;                                                            \
    for (int i = q; i >= p && op == -1; i--) {                                 \
      if (tokens[i].type == '(')                                               \
        parenthese--;                                                          \
      else if (tokens[i].type == ')')                                          \
        parenthese++;                                                          \
      else if (parenthese == 0) {                                              \
        if (tokens[i].type == tok1 || tokens[i].type == tok2) {                \
          op_type = tokens[i].type;                                            \
          op = i;                                                              \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }
    int find_list[20] = {TK_MINUS, TK_NEQ, TK_EQ, TK_AND, '\0'};

    find(TK_DEREF);
    if (op == -1) {
      find2('+', '-');
      if (op == -1) {
        find2('*', '/');
        if (op == -1) {
          for (int j = 0; j < 20; j++) {
            if (find_list[j] == '\0')
              break;
            find(find_list[j]) if (op != -1) break;
          }
        }
      }
    }

    if (op_type == TK_DEREF) {
      word_t val = eval(op + 1, q, success);
      if (*success == false)
        return 0;
      return paddr_read(val, 4);
    }
    if (op_type == TK_MINUS) {
      word_t val = eval(op + 1, q, success);
      if (*success == false)
        return 0;
      return 0 - val;
    }
    // op = the position of 主运算符 in the token expression;
    word_t val1 = eval(p, op - 1, success);
    if (*success == false)
      return 0;
    word_t val2 = eval(op + 1, q, success);
    if (*success == false)
      return 0;
    switch (op_type) {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
      return val1 / val2;
    case TK_NEQ:
      return val1 != val2;
    case TK_EQ:
      return val1 == val2;
    case TK_AND:
      return val1 && val2;
    default:
      assert(0);
    }
  }
}
