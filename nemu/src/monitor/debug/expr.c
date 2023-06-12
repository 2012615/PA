#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_OCT,TK_HEX,TK_NEQ,TK_AND,TK_OR,
  TK_REGS,

  /* TODO: Add more token types */
  //ATTENTION!!!  minus and pointer should be noticed!!!!!!

  TK_MINUS,TK_POINTER

};

/*add part  not sure!!!

struct level{
  int pos;
  int level;
};

//add part   not sure!!!*/

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   * "//" will be treat as one "/"
   */

  //data_type
  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9A-Fa-f]*", TK_HEX},
  {"0|[1-9][0-9]*", TK_OCT},
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REGS},

  //operate
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // sub
  {"\\*", '*'},         // mul
  {"\\/", '/'},         // div
  {"\\(", '('},
  {"\\)", ')'},
  {"!", '!'},           //not
  {"\\&\\&", TK_AND},
  {"\\|\\|", TK_OR},

  //relationship
  {"==", TK_EQ},         // equal    "\\" means multiple spaces??
  {"!=",TK_NEQ}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
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

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
          //Log() can be used to print info, including the position of the code
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        
        //to judge whether the length is greater than 32, if so, shut down.
        if(substr_len>32)
        {
          assert(0);
        }
        //case of space
        if(rules[i].token_type==TK_NOTYPE)
        {
          break;
        }
        else
        {
            tokens[nr_token].type=rules[i].token_type;
            if(tokens[nr_token].type==TK_OCT||tokens[nr_token].type==TK_HEX)
            {
              strncpy(tokens[nr_token].str,substr_start,substr_len);
            }
            if(tokens[nr_token].type==TK_REGS)
            {
              strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
            }
            nr_token++;
            break;
        }

        switch (rules[i].token_type) {
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  //add part not sure!!!

  //there are still minus numbers or pointers

  if(tokens[0].type=='-')
  {
    tokens[0].type=TK_MINUS;
  }
  if(tokens[0].type=='*')
  {
    tokens[0].type=TK_POINTER;
  }
  for(int i=1;i<nr_token;i++)
  {
    if(tokens[i].type=='-')
    {
      if(tokens[i-1].type!=TK_OCT&&tokens[i-1].type!=TK_HEX&&tokens[i-1].type!=TK_REGS&&tokens[i-1].type!=')')
      {
        tokens[i].type=TK_MINUS;
      }
    }
    if(tokens[i].type=='*')
    {
      if(tokens[i-1].type!=TK_OCT&&tokens[i-1].type!=TK_HEX&&tokens[i-1].type!=TK_REGS&&tokens[i-1].type!=')')
      {
        tokens[i].type=TK_POINTER;
      }
    }
  }

  //add part not sure!!!

  return true;
}

bool check_parentheses(int p,int q)
{
  if(p>=q)
  {
    printf("Wrong! NOT THIS CASE! CHECK UR STARTPOINT AND ENDPOINT!\n");
    assert(0);
  }
  else if(tokens[p].type!='('||tokens[q].type!=')')
  {
    return false;
  }
  else
  {
    //make a markpoint, a left one plus, a right one minus.
    int mark=1;
    for(int i=p+1;i<=q;i++)
    {
      if(tokens[i].type=='(')
      {
        mark++;
      }
      if(tokens[i].type==')')
      {
        if(mark==0)
        {
          printf("Wrong! U cannot input a ')'without a '(' in front!!\n");
          assert(0);
        }
        mark--;
      }
    }
    if(mark!=0)//a uncorrect expr
    {
      assert(0);
    }
  }
  return true;
}
//max length is 32
int FindApart(int p,int q)  //check the lowest op
{
  int min=65;  //the grade cannot greater than 33 in 32 bits length
  int grade=0;
  int result=0;
  for(int i=p;i<=q;i++)
  {
    if(tokens[i].type=='(')
    {
      grade+=4;
      continue;
    }
    else if(tokens[i].type==')')
    {
      grade-=4;
      continue;
    }
    else if(tokens[i].type==TK_MINUS||tokens[i].type==TK_POINTER||tokens[i].type=='!')
    {
      if(grade+3<min)
      {
        min=grade+3;
        result=i;
      }
      continue;
    }
    else if(tokens[i].type=='+'||tokens[i].type=='-')
    {
      if(grade+1<=min)
      {
        min=grade+1;
        result=i;
      }
      continue;
    }
    else if(tokens[i].type=='*'||tokens[i].type=='/')
    {
      if((grade+2)<=min)
      {
        min=grade+2;
        result=i;
      }
      continue;
    }
    else if(tokens[i].type==TK_AND||tokens[i].type==TK_OR)
    {
      if(grade<=min)
      {
        min=grade;
        result=i;
      }
      continue;
    }
    else
    {
      continue;
    }
  } 
  return result;
}

int eval(int p,int q)
{
  if(p>q)
  {
    printf("The start position cannot greater than end position!\n");
    assert(0);
  }
  else if(p==q)//means there is just one token in the expr
  {
    if(tokens[p].type==TK_OCT)
    {
      int result;
      sscanf(tokens[p].str,"%d",&result);
      return result;
    }
    else if(tokens[p].type==TK_HEX)
    {
      int result;
      sscanf(tokens[p].str,"%x",&result);
      return result;
    }
    else if(tokens[p].type==TK_REGS)
    {
      for(int i=0;i<8;i++)
      {
        if(strcmp(tokens[p].str,regsl[i])==0)
        {
          return reg_l(i);
        }
        if(strcmp(tokens[p].str,regsw[i])==0)
        {
          return reg_w(i);
        }
        if(strcmp(tokens[p].str,regsb[i])==0)
        {
          return reg_b(i);
        }
      }
      if(strcmp(tokens[p].str,"eip")==0)
        {
          return cpu.eip;
        }
    }
    else
    {
      printf("Wrong token in the expr!\n");
      assert(0);
    }
  }
  else if (check_parentheses(p,q)==true)
  {
    return eval(p+1,q-1); //throw away the '(' and ')'
  }
  else
  {
    int ope=FindApart(p,q);
    vaddr_t addr;
    int final;
    if(tokens[ope].type==TK_MINUS)
    {
      return -eval(ope+1,q);
    }
    else if(tokens[ope].type==TK_POINTER)
    {
      addr=eval(ope+1,q);
      final=vaddr_read(addr,4);
      return final;
    }
    else if(tokens[ope].type=='!')
    {
      return eval(ope+1,q)>0?1:0;
    }
    else
    {
      int num1,num2;
      num1=eval(p,ope-1);
      num2=eval(ope+1,q);
      switch(tokens[ope].type)
      {
        case '+':
          return num1+num2;
          break;
        case '-':
          return num1-num2;
          break;
        case '*':
          return num1*num2;
          break;
        case '/':
          return num1/num2;
          break;
        case TK_EQ:
          return num1==num2;
          break;
        case TK_NEQ:
          return num1!=num2;
          break;
        case TK_AND:
          return num1&&num2;
          break;
        case TK_OR:
          return num1||num2;
          break;
        default:
          printf("No such a opeator!!\n");
          assert(0);
      }
    }
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //Nothing to do?
  //TODO();

  *success=true;
  return eval(0,nr_token-1);
}

