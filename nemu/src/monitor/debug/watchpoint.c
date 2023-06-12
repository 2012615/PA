#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char* args) //choose a point in the beginning of the list?
{  //should send the expr
  if(free_==NULL)
  {
    printf("Error! There is no free WP!\n");
    assert(0);
  }
  WP* se=free_;
  free_=free_->next;
  strcpy(se->TheExpr,args);
  bool success;
  se->OldVal=expr(args,success);
  if(!success)
  {
    printf("Something wrong in calculating\n");
    assert(0);
  }

  if(head==NULL)
  {
    head=se;
    head->next=NULL;
  }
  else
  {
    WP* current=head;
    while(current->next!=NULL)
    {
      current=current->next;
    }
    current->next=se;
    se->next=NULL;
  }
  printf("Successfully set WP %d, the expr is %s, and the OldVal of the expr is %d\n",
  se->NO,se->TheExpr,se->OldVal);
  return se;
}

void free_wp(int no)
{
  if(no<0||no>=32)
  {
    printf("Wrong! check ur wp number!!\n");
    assert(0);
  }
  if(head==NULL)
  {
    printf("No WP-in-use.\n");
    return;
  }
  WP* current=head;
  if(current->NO==no)
  {
    WP* se=current;
    head=head->next;
    se->next=free_;
    free_=se;
  }
  else
  {
    while(current->next!=NULL)
    {
      if(current->next->NO==no&&current->next!=NULL)
      {
        WP* se=current->next;
        current->next=current->next->next;
        se->next=free_;
        free_=se;
        printf("WP %d deleted.\n",se->NO);
        break;
      }
      current=current->next;
    }
    if(current->next==NULL)
    {
      printf("No such a wp!\n");
    }
  }
}

void WPrint()
{
  if(head==NULL)
  {
    printf("There is no WP-in-use.\n");
    return;
  }
  WP*current=head;
  while(current!=NULL)
  {
    printf("%d\t%s\t\t%d\n",current->NO,current->TheExpr,current->OldVal);
    current=current->next;
  }
}

bool Check()
{
  bool success;
  int result;
  if(head==NULL)
  {
    return true;
  } 
  WP*current=head;
  while(current!=NULL)
  {
    result=expr(current->TheExpr,&success);
    if(!success)
    {
      printf("Something wrong in the calculating!\n");
      assert(0);
    }
    if(result==current->OldVal)
    {
      continue;
    }
    else
    {
      printf("Watchpoint %d: %s\n",current->NO,current->TheExpr);
      printf("OldVal: %d\t\tNewVal: %d",current->OldVal,result);
      return false;
    }
  }
  return true;
}
