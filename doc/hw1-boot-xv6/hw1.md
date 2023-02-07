# Homework 1: boot xv6

## Boot xv6

Get xv6 source:

```shell
git clone git://github.com/mit-pdos/xv6-public.git
```



## Finding and breaking at an address

Find the address of `_start`, the entry point of the kernel:

```
$ nm kernel | grep _start
8010a48c D _binary_entryother_start
8010a460 D _binary_initcode_start
0010000c T _start
```

In this case, the address is `0010000c`.

Run the kernel inside QEMU GDB, setting a breakpoint at `_start` (i.e., the address you just found).

```shell
# in one shell:
make qemu-nox-gdb

# in another shell:
gdb

```

## Exercise: What is on the stack?

- Begin by restarting qemu and gdb, and set a break-point at 0x7c00, the start of the boot block (bootasm.S). Single step through the instructions    (type `si` at the gdb prompt). Where in bootasm.S is the stack pointer    initialized?  (Single step until you see an instruction that moves a value    into `%esp`, the register for the stack pointer.)

  ```
    # Set up the stack pointer and call into C.
    movl    $start, %esp
      7c43:	bc 00 7c 00 00       	mov    $0x7c00,%esp
    call    bootmain
      7c48:	e8 dd 00 00 00       	call   7d2a <bootmain>
  ```

  From Above, we can see that the bootloader set the initial stack starting at 0x7c00. Then we set a breakpoint at `0x7c43` according to previous result

  ```
  (gdb) br *0x7c43
  Breakpoint 1 at 0x7c43
  (gdb) c
  Continuing.
  The target architecture is set to "i386".
  => 0x7c43:      mov    $0x7c00,%esp
  
  Thread 1 hit Breakpoint 1, 0x00007c43 in ?? ()
  (gdb) info reg
  eax            0x0                 0
  ecx            0x0                 0
  edx            0x80                128
  ebx            0x0                 0
  esp            0x6f20              0x6f20
  ebp            0x0                 0x0
  esi            0x0                 0
  edi            0x0                 0
  eip            0x7c43              0x7c43
  eflags         0x6                 [ PF ]
  cs             0x8                 8
  ss             0x10                16
  ds             0x10                16
  es             0x10                16
  fs             0x0                 0
  gs             0x0                 0
  (gdb) x/24x $esp
  0x6f20: 0xf000d239      0x00000000      0x00006f62      0x00000000
  0x6f30: 0x00008d0d      0x00008cd4      0x00000000      0x00000000
  0x6f40: 0x00006ae5      0x00007c00      0x00007c00      0x00000080
  0x6f50: 0x000f1c5d      0x000f3c9a      0x00000000      0x00007c00
  0x6f60: 0x00000000      0x00000000      0x00000000      0x00000000
  0x6f70: 0x00000000      0x00800000      0x00000000      0xaa550000
  (gdb) si
  => 0x7c48:      call   0x7d2a
  0x00007c48 in ?? ()
  (gdb) info reg
  eax            0x0                 0
  ecx            0x0                 0
  edx            0x80                128
  ebx            0x0                 0
  esp            0x7c00              0x7c00
  ebp            0x0                 0x0
  esi            0x0                 0
  edi            0x0                 0
  eip            0x7c48              0x7c48
  eflags         0x6                 [ PF ]
  cs             0x8                 8
  ss             0x10                16
  ds             0x10                16
  es             0x10                16
  fs             0x0                 0
  gs             0x0                 0
  (gdb) x/24x $esp
  0x7c00: 0x8ec031fa      0x8ec08ed8      0xa864e4d0      0xb0fa7502
  0x7c10: 0xe464e6d1      0x7502a864      0xe6dfb0fa      0x16010f60
  0x7c20: 0x200f7c78      0xc88366c0      0xc0220f01      0x087c31ea
  0x7c30: 0x10b86600      0x8ed88e00      0x66d08ec0      0x8e0000b8
  0x7c40: 0xbce88ee0      0x00007c00      0x0000dde8      0x00b86600
  0x7c50: 0xc289668a      0xb866ef66      0xef668ae0      0x9066feeb
  (gdb)
  ```

   As we can see,  the top of stack is pointing to `0x7c00`

- Single step through the call to `bootmain`; what is on the stack now?

  > As we can see from the previous, the next instruction is calling  `bootmain` . So, we can just step one more.
  >
  > ```
  > (gdb) si
  > => 0x7d2a:      push   %ebp
  > 0x00007d2a in ?? ()
  > (gdb) info reg
  > eax            0x0                 0
  > ecx            0x0                 0
  > edx            0x80                128
  > ebx            0x0                 0
  > esp            0x7bfc              0x7bfc
  > ebp            0x0                 0x0
  > esi            0x0                 0
  > edi            0x0                 0
  > eip            0x7d2a              0x7d2a
  > eflags         0x6                 [ PF ]
  > cs             0x8                 8
  > ss             0x10                16
  > ds             0x10                16
  > es             0x10                16
  > fs             0x0                 0
  > gs             0x0                 0
  > (gdb) x/24x $esp
  > 0x7bfc: 0x00007c4d      0x8ec031fa      0x8ec08ed8      0xa864e4d0
  > 0x7c0c: 0xb0fa7502      0xe464e6d1      0x7502a864      0xe6dfb0fa
  > 0x7c1c: 0x16010f60      0x200f7c78      0xc88366c0      0xc0220f01
  > 0x7c2c: 0x087c31ea      0x10b86600      0x8ed88e00      0x66d08ec0
  > 0x7c3c: 0x8e0000b8      0xbce88ee0      0x00007c00      0x0000dde8
  > 0x7c4c: 0x00b86600      0xc289668a      0xb866ef66      0xef668ae0
  > (gdb)
  > ```
  >
  > We can find that, $esp has been reduced by 4, and `0x7c4d` is putted in, which is corresponding to the instruction following the call to `bootmain`
  >
  > ```
  >   # Set up the stack pointer and call into C.
  >   movl    $start, %esp
  >     7c43:	bc 00 7c 00 00       	mov    $0x7c00,%esp
  >   call    bootmain
  >     7c48:	e8 dd 00 00 00       	call   7d2a <bootmain>
  > 
  >   # If bootmain returns (it shouldn't), trigger a Bochs
  >   # breakpoint if running under Bochs, then loop.
  >   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
  >     7c4d:	66 b8 00 8a          	mov    $0x8a00,%ax
  > ```

- What do the first assembly instructions of bootmain do to the stack? Look for bootmain in bootblock.asm.

  > From previous, the next instruction after calling `bootmain` is `push %ebp`, which is the convention of gcc c calling, we save the previous frame's $ebp register.
  >
  > Let's find it in bootblock.asm:
  >
  > ```
  > void
  > bootmain(void)
  > {
  >     7d2a:	55                   	push   %ebp
  >     7d2b:	89 e5                	mov    %esp,%ebp
  >     7d2d:	57                   	push   %edi
  >     7d2e:	56                   	push   %esi
  >     7d2f:	53                   	push   %ebx
  >     7d30:	83 ec 2c             	sub    $0x2c,%esp
  > ```

- Continue tracing via gdb (using breakpoints if necessary -- see hint below) and look for the call that changes `eip` to `0x10000c`. What does    that call do to the stack? (Hint: Think about what this call is trying to    accomplish in the boot sequence and try to identify this point in bootmain.c, and the corresponding instruction in the bootmain code in bootblock.asm. This might help you set suitable breakpoints to speed things up.)

  > We display the content of $esp, and what the content is pointing to. After calling bootmain.
  >
  > ```
  > (gdb) br *0x7d2a
  > Breakpoint 1 at 0x7d2a
  > (gdb) c
  > Continuing.
  > The target architecture is set to "i386".
  > => 0x7d2a:      push   %ebp
  > 
  > Thread 1 hit Breakpoint 1, 0x00007d2a in ?? ()
  > (gdb) display $esp
  > 1: $esp = (void *) 0x7bfc
  > (gdb) display/x *( (int *) $esp )
  > 2: /x *( (int *) $esp ) = 0x7c4d
  > (gdb) si
  > => 0x7d2b:      mov    %esp,%ebp
  > 0x00007d2b in ?? ()
  > 1: $esp = (void *) 0x7bf8
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d2d:      push   %edi
  > 0x00007d2d in ?? ()
  > 1: $esp = (void *) 0x7bf8
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d2e:      push   %esi
  > 0x00007d2e in ?? ()
  > 1: $esp = (void *) 0x7bf4
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d2f:      push   %ebx
  > 0x00007d2f in ?? ()
  > 1: $esp = (void *) 0x7bf0
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d30:      sub    $0x2c,%esp
  > 0x00007d30 in ?? ()
  > 1: $esp = (void *) 0x7bec
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d33:      movl   $0x0,0x8(%esp)
  > 0x00007d33 in ?? ()
  > 1: $esp = (void *) 0x7bc0
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d3b:      movl   $0x1000,0x4(%esp)
  > 0x00007d3b in ?? ()
  > 1: $esp = (void *) 0x7bc0
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d43:      movl   $0x10000,(%esp)
  > 0x00007d43 in ?? ()
  > 1: $esp = (void *) 0x7bc0
  > 2: /x *( (int *) $esp ) = 0x0
  > (gdb)
  > => 0x7d4a:      call   0x7ce6
  > 0x00007d4a in ?? ()
  > 1: $esp = (void *) 0x7bc0
  > 2: /x *( (int *) $esp ) = 0x10000
  > (gdb) x/24x $esp
  > 0x7bc0: 0x00010000      0x00001000      0x00000000      0x00000000
  > 0x7bd0: 0x00000000      0x00000000      0x00000000      0x00000000
  > 0x7be0: 0x00000000      0x00000000      0x00000000      0x00000000
  > 0x7bf0: 0x00000000      0x00000000      0x00000000      0x00007c4d
  > 0x7c00: 0x8ec031fa      0x8ec08ed8      0xa864e4d0      0xb0fa7502
  > 0x7c10: 0xe464e6d1      0x7502a864      0xe6dfb0fa      0x16010f60
  > (gdb)
  > 
  > ```
  >
  > We can now find that the bootmain first push $ebp, $edi, $esi, $ebx onto stack, and then reserve the space for local variables, which are also used as input arguments of callee function. We now have:
  >
  > ```
  > (gdb) x/24x $esp
  > 0x7bc0: 0x00010000      0x00001000      0x00000000      0x00000000
  > //		1st arg			2nd arg			3rd of readseg	Reserved Space
  > 0x7bd0: 0x00000000      0x00000000      0x00000000      0x00000000
  > //		<-					reserved space						->
  > 0x7be0: 0x00000000      0x00000000      0x00000000      0x00000000
  > //		<-				reserved space			->		saved $ebx
  > 0x7bf0: 0x00000000      0x00000000      0x00000000      0x00007c4d
  > //		saved $esi		saved $edi		saved $ebp		return IP
  > 0x7c00: 0x8ec031fa      0x8ec08ed8      0xa864e4d0      0xb0fa7502
  > 0x7c10: 0xe464e6d1      0x7502a864      0xe6dfb0fa      0x16010f60
  > (gdb)
  > ```
  >
  > From `bootmain.c` we have:
  >
  > ```C
  >   elf = (struct elfhdr*)0x10000;  // scratch space
  > ```
  >
  > So, the elf file is loaded from 0x10000. From [elf format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) we know that the entry is located at offset `0x18`. Therefore `0x10018` is where the first instruction's address of kernel. In `bootlock.asm`, we can find that the `bootmain.c` indeed enter the entry at `0x10018`
  >
  > ```
  >   // Call the entry point from the ELF header.
  >   // Does not return!
  >   entry = (void(*)(void))(elf->entry);
  >   entry();
  >     7db2:	ff 15 18 00 01 00    	call   *0x10018
  > ```
  >
  > Then, we break at `0x7db2` and examine what is on the stack
  >
  > ```
  > (gdb) br *0x7db2
  > Breakpoint 2 at 0x7db2
  > (gdb) c
  > Continuing.
  > => 0x7db2:      call   *0x10018
  > 
  > Thread 1 hit Breakpoint 2, 0x00007db2 in ?? ()
  > 1: $esp = (void *) 0x7bc0
  > 2: /x *( (int *) $esp ) = 0x107000
  > (gdb) info r
  > eax            0x0                 0
  > ecx            0x0                 0
  > edx            0x1f0               496
  > ebx            0x10074             65652
  > esp            0x7bc0              0x7bc0
  > ebp            0x7bf8              0x7bf8
  > esi            0x107000            1077248
  > edi            0x1144a8            1131688
  > eip            0x7db2              0x7db2
  > eflags         0x46                [ PF ZF ]
  > cs             0x8                 8
  > ss             0x10                16
  > ds             0x10                16
  > es             0x10                16
  > fs             0x0                 0
  > gs             0x0                 0
  > (gdb) x/24x $esp
  > 0x7bc0: 0x00107000      0x00002516      0x00008000      0x00000000
  > 0x7bd0: 0x00000000      0x00000000      0x00000000      0x00010074
  > 0x7be0: 0x00000000      0x00000000      0x00000000      0x00000000
  > 0x7bf0: 0x00000000      0x00000000      0x00000000      0x00007c4d
  > 0x7c00: 0x8ec031fa      0x8ec08ed8      0xa864e4d0      0xb0fa7502
  > 0x7c10: 0xe464e6d1      0x7502a864      0xe6dfb0fa      0x16010f60
  > (gdb) x/x *0x10018
  > 0x10000c:       0x83e0200f
  > (gdb) print *0x10018
  > $1 = 1048588
  > (gdb) print/x *0x10018
  > $2 = 0x10000c
  > (gdb)
  > ```
  >
  > As we can see, the bottom of stack is pointing to `0x7bc0`, which is the same as it back to the beginning of the bootmain function. Only reserved space have been changed, saved variables remain unchanged. Also, the content of address `0x10018` is pointing at `0x10000c`, which is the first instruction of kernel. We step further to say what happend:
  >
  > ```
  > (gdb) si
  > => 0x10000c:    mov    %cr4,%eax
  > 0x0010000c in ?? ()
  > 1: $esp = (void *) 0x7bbc
  > 2: /x *( (int *) $esp ) = 0x7db8
  > (gdb) info r
  > eax            0x0                 0
  > ecx            0x0                 0
  > edx            0x1f0               496
  > ebx            0x10074             65652
  > esp            0x7bbc              0x7bbc
  > ebp            0x7bf8              0x7bf8
  > esi            0x107000            1077248
  > edi            0x1144a8            1131688
  > eip            0x10000c            0x10000c
  > eflags         0x46                [ PF ZF ]
  > cs             0x8                 8
  > ss             0x10                16
  > ds             0x10                16
  > es             0x10                16
  > fs             0x0                 0
  > gs             0x0                 0
  > (gdb) x/24x $esp
  > 0x7bbc: 0x00007db8      0x00107000      0x00002516      0x00008000
  > 0x7bcc: 0x00000000      0x00000000      0x00000000      0x00000000
  > 0x7bdc: 0x00010074      0x00000000      0x00000000      0x00000000
  > 0x7bec: 0x00000000      0x00000000      0x00000000      0x00000000
  > 0x7bfc: 0x00007c4d      0x8ec031fa      0x8ec08ed8      0xa864e4d0
  > 0x7c0c: 0xb0fa7502      0xe464e6d1      0x7502a864      0xe6dfb0fa
  > (gdb)
  > ```
  >
  > As we can see, return IP is pushed, although we will never jump to that position.
  >
  > We can examine what is the instruction at `0x10000c` in `kernel.asm`
  >
  > ```
  > # Entering xv6 on boot processor, with paging off.
  > .globl entry
  > entry:
  >   # Turn on page size extension for 4Mbyte pages
  >   movl    %cr4, %eax
  > 8010000c:	0f 20 e0             	mov    %cr4,%eax
  > 
  > ```
  >
  > Note that at that time, we have already jump to high address, but the kernel is located at low physical address.
  >
  > Thus, we have the final answer, as we already figured it out:
  >
  > ```
  > 0x7bc0: 0x00010000      0x00001000      0x00000000      0x00000000
  > //		1st arg			2nd arg			3rd of readseg	Reserved Space
  > 0x7bd0: 0x00000000      0x00000000      0x00000000      0x00000000
  > //		<-					reserved space						->
  > 0x7be0: 0x00000000      0x00000000      0x00000000      0x00000000
  > //		<-				reserved space			->		saved $ebx
  > 0x7bf0: 0x00000000      0x00000000      0x00000000      0x00007c4d
  > //		saved $esi		saved $edi		saved $ebp		return IP
  > 0x7c00: 0x8ec031fa      0x8ec08ed8      0xa864e4d0      0xb0fa7502
  > //		<- 				bootblock code 							->
  > 0x7c10: 0xe464e6d1      0x7502a864      0xe6dfb0fa      0x16010f60
  > //		<- 				bootblock code 							->
  > ```



