#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL

extern pte_t* walk(pagetable_t, uint64, int); 

int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 pg;
  argaddr(0, &pg);
  int num;
  argint(1, &num);
  if(num>64)
    return -1;
  uint64 buff;
  argaddr(2, &buff);

  uint64 res = 0;
  struct proc *p = myproc();
  int count = 0;
  for(uint64 p_t = pg; p_t < pg+num*PGSIZE; p_t+=PGSIZE){
    pte_t *pte;
    pte = walk(p->pagetable, p_t, 0);
    if(*pte & PTE_A){
      res = res|(1 << count);
      *pte = (*pte) & (~PTE_A);
    }
    count++;
  }


  if(copyout(p->pagetable, buff, (char*)&res,sizeof(res))<0)
    return -1;
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

