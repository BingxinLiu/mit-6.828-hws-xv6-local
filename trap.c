#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
// #include <sys/types.h>

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    if(myproc() != 0 && (tf->cs & 3) == 3)
    {
      if (myproc()->alarmticks > 0)
        myproc()->passedticks++;
      if (myproc()->alarmhandler_returned
              && myproc()->alarmticks > 0
              && myproc()->passedticks >= myproc()->alarmticks)
      {
        myproc()->passedticks = 0;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->eip;
        // Adde for optional challenge
        // tf->esp -= 4;
        // *(uint*)(tf->esp) = 0x90c308c4;
        // tf->esp -= 4;
        // *(uint*)(tf->esp) = 0x83585a59;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->eax;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->edx;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->ecx;
        // Add for jump to restore caller-saved registers
        tf->esp -= 4;
        *(uint*)(tf->esp) = 0x308;
        // tf->esp -= 4;
        // *(uint*)(tf->esp) = tf->esp+0x10;
        // Add end, code injection
        tf->eip = (uint)(myproc()->alarmhandler);
        myproc()->alarmhandler_returned = 0;
      }
      // debug
      // else {
      //   cprintf("alarmticks: %d; alarm passed ticks: %d.\n", myproc()->alarmticks, myproc()->passedticks);
      // }

    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }

    if(tf->trapno == T_PGFLT) {
      uint addr = rcr2();
      char *mem = kalloc();
      if (mem != 0) {
        if(myproc()->sz > addr && (addr + PGSIZE) > addr) {
          int mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);
          if(mappages(myproc()->pgdir, (void *) PGROUNDDOWN(addr), PGSIZE, V2P(mem), PTE_W | PTE_U) == 0) 
            return;
          else
            cprintf("mappages error\n");
        } else {
          cprintf("address overflow or access limited address\n");
        }
      }
      else {
        cprintf("use out of memor\n");
      }
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
