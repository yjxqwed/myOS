# Notes

## Development Environment
### WSL (WSL2 on Win10 build 2004)
### X server
> Download and install <a href="https://sourceforge.net/projects/vcxsrv/">XLaunch</a> on Windows  
> export DISPLAY="`grep nameserver /etc/resolv.conf | sed 's/nameserver //'`:0"
### Bochs
> sudo apt update  
> sudo apt upgrade  
> sudo apt install build-essential bison xorg-dev g++  
> sudo apt install bochs-x  
> Download and extract <a href="http://bochs.sourceforge.net/">bochs</a>  
> cd {directory of bochs}  
> ./configure  --enable-debugger --enable-disasm  
> sudo make && sudo make install  
### Compile the OS
> sudo apt install g++ nasm binutils grub-pc-bin xorriso
### Run
> bochs -q  


## Temp Memory Layout of myOS

<pre>
|           |   unused in myOS (low 1MiB memory)
+-----------+ ------------------------------------------------
|           |   0x0010 0000
|  kernel   |
|  binary   |                                         6 MiB
|           |   0x006f ffff
+-----------+ -------------------------------------------------
|           |   0x0070 0000
|    GDT    |
|    032    |     256 B (32 x 8 B)
|  entries  |
|           |   0x0070 00ff
+-----------+
|           |   0x0070 0100
|    TSS    |     512 B (104 B actually)
|           |   0x0070 02ff                           1 MiB
+-----------+
|           |   0x0070 0300
|    IDT    |
|    256    |     2 KiB (256 x 8B)
|  entries  |
|           |   0x0070 0aff
+-----------+
|           |   0x0070 0b00
|  kppool   |    0x1c KiB (0x38000 p-pages at most)
|   btmp    |   0x0070 7aff
+-----------+
|           |   0x0070 7b00
|  uppool   |    0x64 KiB (0xc8000 p-pages at most)
|   btmp    |   0x0072 0aff
+-----------+
|   free    |   0x0072 0b00 ~ 0x007f ffff
+-----------+ --------------------------------------------------
|           |   0x0080 0000
|    PD     |      4 KiB
|           |   0x0080 0fff
+-----------+                                         1 MiB
|           |   0x0080 1000
|   kernel  |      255 * 4 KiB
|    PTS    |
|           |   0x008f ffff
+-----------+ --------------------------------------------------
|           |   0x0090 0000
|  k_stack  |                                         4 MiB
|           |   0x00cf ffff
+-----------+
|           |
</pre>

## Boot the computer
### BIOS  
After the computer is powered on, the hardware will automatically load the BIOS into the memory and jump to the start point of the BIOS (by seeting cs:ip = 0xF000:0xFFF0). The BIOS will perform many operations like POST (Power On Self Test) and will try to load the boot sector from bootable devices (floppy, hard disk, optical and etc)

### MBR (Master Boot Record)  
MBR is a 512B executable ending in 0x55AA (a magic number that tells the BIOS that this is an MBR). After finding an MBR, the BIOS will load it to 0x7c00 in the memory and jump to 0x7c00 (the entry point of the bootloader).

### Real Mode and Protected Mode  
Trying to be compatible with the 8086 CPU, which works under the 16-bit mode, CPUs like 80386 still works under the 16-bit mode after powered on although they are actually 32-bit cpus (more modern cpus are 64-bit).

Under the Real Mode, registers are 16-bit and the cpu address bus is 20-bit, it can use 1MB of the memory (Physical addr = seg * 10h + offset, seg and offset are both 16-bit). Under this mode, there is no protection, any program can easily undermine the OS, so 32-bit Protected Mode is used in the later cpus.

#### Protected Mode
##### Segment
The segment:offset structure is still used while the segment is an index to the segment descriptor and the offset is now 32-bit. Segment descriptors are stored in a table called the Global Descriptor Table (<a href="https://wiki.osdev.org/GDT">GDT</a>).

##### Interrupts
To handle the interrupts, we need a Interrupt Descriptor Table (<a href="https://wiki.osdev.org/IDT">IDT</a>). Entries in this table are so-called Gate Descriptors. There are 4 types of Gate descriptors:
* Call Gate
* Interrupt Gate
* Trap Gate
* Task Gate

A call gate is used to let a program in the lower privileged level (ring 3) can call functions in a higher privileged level (ring 0). A function A whats to call function B via a call gate G should satisfy the privilege Check max(CPL_A, RPL_A) <= DPL_G && CPL_A >= DPL_B. Call gate is used to raise the privilege.

An interrupt/trap gate is used to call the interrupt service routines (ISRs) from ring 0 or ring 3. The only difference is the interrupt gate will clear the IF flag wile the trap gate won't.

##### Exceptions
Generally, interrupts are a subset of the <a href="https://wiki.osdev.org/Exceptions">exceptions</a>. When an exception is raised, the cpu will push the return address and many other registers. The return address is determined by the exception type.

* int 0-31: Intel Defined CPU exceptions
> Fault: return address = this instruction  
> Trap: return address = next instruction  
> Abort: no return address
* int 32-128: User Defined interrupts
> interrupts: return address = next instruction


##### Enter Ring 3
Using gates can raise the privilege while there is only one way to lower the privilege: return from a interrupt handler.


##### Enter the Protected Mode
* Enable the A20 Line
* Load the GDT
* Set bit 0 of the cr0 register

The <a href="https://wiki.osdev.org/GRUB">GRUB</a> will do this for us. But we still need to override the GDT with the lgdt instruction.

## Memory Management
### Paging

#### x86 Paging 
The x86 cpu uses 2-level paging. The 32-bit address is used as 3 parts, as below.
<pre>
| <-------- 32 bits --------->|
| 10 bits | 10 bits | 12 bits |
    |         |         |
    +---------(---------(------ page directory index
              |         |
              +---------(------ page table index
                        |
                        +------ page frame offset
</pre>
#### Higher Half
The x86 system supports an address space of 4-GiB; for reasons of compatibility and linker convention, the kernel space is mapped to the higer part of the virtual address space (above 0xc000 0000 in myOS); this is called a higher half kernel.

#### Enable Paging
To enable Paging, we need to do the following
* Setup page directory and page tables (at least for the kernel)
* Load page directory address to the cr3 register
* Set bit 32 (paging-enabling bit) of the cr0 register

Some tips:
* The kernel should be compiled starting at 0xc000 0000 but must be loaded at the lower part of the memory, we need to use linker script directives . and AT to do this (. is for indicating the compiling starting address; AT is for grub to know where in the mem to put the binary).
* At the very beginning, we should make an extra identity map of the lower part of the memory. After enabling paging, we should jump to the higer space and clear the identity map.