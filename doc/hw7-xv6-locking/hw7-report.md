# Homework7: xv6 locking

## Don't do this

```c
  struct spinlock lk;
  initlock(&lk, "test lock");
  acquire(&lk);
  acquire(&lk);
```

> Submit: Explain in one sentence what happens. 

The kernel will deadlock because it tries to acquire a locked lock.

## Interrupts in ide.c

Let's see what happens if we turn on interrupts while holding the ide lock. In iderw in ide.c, add a call to sti() after the acquire(), and a call to cli() just before the release(). Rebuild the kernel and boot it in QEMU. Chances are the kernel will panic soon after boot; try booting QEMU a few times if it doesn't. 

> Submit: Explain in a few sentences why the kernel panicked. You may find it useful to look up the stack trace (the sequence of %eip values printed by panic) in the kernel.asm listing.

Modify iderw like below:

```c
void
iderw(struct buf *b)
{
  struct buf **pp;

  if(!holdingsleep(&b->lock))
    panic("iderw: buf not locked");
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  if(b->dev != 0 && !havedisk1)
    panic("iderw: ide disk 1 not present");

  acquire(&idelock);  //DOC:acquire-lock
  // hw7
  sti();

  // Append b to idequeue.
  b->qnext = 0;
  for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
    ;
  *pp = b;

  // Start disk if necessary.
  if(idequeue == b)
    idestart(b);

  // Wait for request to finish.
  while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
    sleep(b, &idelock);
  }

  // hw7
  cli();

  release(&idelock);
}
```

Run the kernel, we got:

```shell
$ qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 2 -m 512
xv6...
cpu1: starting 1
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
lapicid 1: panic: sched locks
 8010369a 801037d7 80105213 80104ed5 80101f9e 80102738 801027d8 80103206 80104ed8 0
QEMU: Terminated
$
```

This is because we enable interrupt after calling acquire in `iderw`, and now ncli is 1 added in function `pushcli`, then the machine may raise an interrupt, in the corresponding routine, it also calls `iderw` and increment ncli by 1 in function `pushcli`. Then, when scheduling, function `sched` is called, and it checks if ncli is equal to 1. However, at that time, ncli already equals to 2, which causes panic

## Interrupts in file.c

Now let's see what happens if we turn on interrupts while holding the file_table_lock. This lock protects the table of file descriptors, which the kernel modifies when an application opens or closes a file. In filealloc() in file.c, add a call to sti() after the call to acquire(), and a cli() just before each of the release()es. You will also need to add #include "x86.h" at the top of the file after the other #include lines. Rebuild the kernel and boot it in QEMU. It most likely will not panic.

> Submit: Explain in a few sentences why the kernel didn't panic. Why do file_table_lock and ide_lock have different behavior in this respect?
> 
> You do not need to understand anything about the details of the IDE hardware to answer this question, but you may find it helpful to look at which functions acquire each lock, and then at when those functions get called.

It seems like only one thread accesses this critical area

## xv6 lock implementation

> Submit: Why does release() clear lk->pcs[0] and lk->cpu before clearing lk->locked? Why not wait until after?

Other threads may acquire the lock before cleaning `lk->pcs[0]` and `lk->cpu` and gets wrong content.
