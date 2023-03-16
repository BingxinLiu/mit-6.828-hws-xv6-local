# Challenge

```c

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
      if (myproc()->alarmticks > 0 && myproc()->passedticks >= myproc()->alarmticks)
      {
        myproc()->passedticks = 0;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->eip;
        // Adde for optional challenge
        tf->esp -= 4;
        *(uint*)(tf->esp) = 0x90c308c4;
        tf->esp -= 4;
        *(uint*)(tf->esp) = 0x83585a59;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->eax;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->edx;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->ecx;
        tf->esp -= 4;
        *(uint*)(tf->esp) = tf->esp+0x10;
        // Add end, code injection
        tf->eip = (uint)(myproc()->alarmhandler);
      }
      // debug
      // else {
      //   cprintf("alarmticks: %d; alarm passed ticks: %d.\n", myproc()->alarmticks, myproc()->passedticks);
      // }

    }
    lapiceoi();
    break;

```

usys.S:
```assembly
.globl __restore_caller_saved_registers
__restore_caller_saved_registers:
  popl %ecx
  popl %edx
  popl %eax
  add $0x8,%esp
  ret
```

user.h:
```
// export __caller_saved_registers:
int __caller_saved_registers(void);
```

alarmtest.asm:
```assembly
00000308 <__restore_caller_saved_registers>:

.globl __restore_caller_saved_registers
__restore_caller_saved_registers:
  popl %ecx
 308:	59                   	pop    %ecx
  popl %edx
 309:	5a                   	pop    %edx
  popl %eax
 30a:	58                   	pop    %eax
  add $0x8,%esp
 30b:	83 c4 08             	add    $0x8,%esp
  ret
 30e:	c3                   	ret    
 30f:	90                   	nop
```

