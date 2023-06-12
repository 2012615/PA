#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
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
//continue the processing
static int cmd_q(char *args) {
  return -1;
}
//quit the debugger
static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_p(char *args);

static int cmd_d(char*args);

static int cmd_x(char*args);

static int cmd_w(char*args);


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help},
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  
  { "si", "args: [N]; to run N steps", cmd_si},
  { "info", "args: r/w; print the info of registers or watchpoints", cmd_info},
  { "x","[N] and [expr]; to scan the memory from expr to expr+N", cmd_x},
  { "p", "args:expr; to calculate the value of expr", cmd_p},
  { "d", "args:N; to delete the watchpoint whose NO is N", cmd_d},
  { "w", "args:expr; to set up a new WatchPoint", cmd_w}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

//not sure!!!!
static int cmd_si(char* args) //use ssanf to read from the param (str,cond,str)
{
	uint64_t steps=0;
	if(args==NULL)
	{
		steps=1;
	}
	else
	{
		int flag=sscanf(args,"%lu",&steps);  /*llu means a unsigned long integer(64 bits)*/
		if(flag<=0)
		{
			printf("wrong param!\n");
			return 0;
		}
	}
	cpu_exec(steps);
	return 0;
}
//not sure!!1

static int cmd_p(char *args)
{
  bool success;
  int result=expr(args,&success);
  if(!success)
  {
    printf("Something Wrong in the expr!\n");
  }
  else
  {
    printf("Result: %d\n",result);
  }
  return 0;
}

static int cmd_x(char*args)
{
	int N;
  int Nsize;  //size of n
  for(int i=0;args[i]!=' ';i++)
  {
    Nsize++;
  }
	char*Expr=(char*)malloc(strlen(args+Nsize+1));//The length of the expr
  int flag;
  flag=sscanf(args,"%d %s",&N,Expr);
  if(flag<=0)
  {
    printf("Incorrect param!\n");
    assert(0);
  }

  bool success;
  vaddr_t addr=expr(Expr,&success);
  if(!success)
  {
    printf("Something wrong in expr.\n");
    return 0;
  }

  printf("Memory from %d(0x%x)\n",addr,addr);
  for(int i=0;i<N;i++)
  {
    //ATTENTION!!! THE OUTPUT FORM!!!
    if(i%4==0)
    {
      printf("\n0x%x:  0x%02x",addr+i,vaddr_read(addr+i,1));
    }
    else
    {
      printf("  0x%02x",vaddr_read(addr+i,1));
    }
  }
  printf("\n");
	return 0;
}

static int cmd_info(char*args)
{
	if(args==NULL)
	{
		printf("no param!\n");
		return 0;
	}
	char opt;

	int flag=sscanf(args,"%c",&opt);
	if(flag<=0)
	{
		printf("Error in scan!\n");
	}
    
	if(opt=='r')
	{
		//according to reg.h, we can just print all the registers we have
		for(int i=0;i<8;i++)
		{
			printf("%s 0x%x\n",regsl[i],reg_l(i));
		}
		//then need to print eip-->ATTENTION!!!
		printf("eip 0x%x\n",cpu.eip);

		for(int i=0;i<8;i++)
		{
			printf("%s 0x%x\n",regsw[i],reg_w(i));
		}

		for(int i=0;i<8;i++)
		{
			printf("%s 0x%X\n",regsb[i],reg_b(i));
		}

		//cr0 and cr3 should be printed-->ATTENTION
		//cr0:to deal with the operation mode and status
		//cr1:not use  cr2:the linear address that make the page error(or page fault)
		//cr3:PDBR  the base address of the PD
		/*printf("CR0 0x%x\n",cpu.CR0);
		printf("CR3 0x%x\n",cpu.CR3);*/
		return 0;
	}

	else if(opt=='w')
	{
		//todo:print the info of watchpoints

	}

	else
	{
		printf("Wrong param!\n");
	}

	return 0;	
}


static int cmd_w(char*args)
{

  return 0;
}

static int cmd_d(char*args)
{
  int No;
  int flag=sscanf(args,"%d",&No);
  if(flag<=0)
  {
    printf("Wrong param!\n");
    return 0;
  }


  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
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

