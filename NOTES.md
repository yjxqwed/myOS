# Notes

## Development Environment

### Compiling Bochs
`sudo apt install build-essential bison xorg-dev g++ bochs-x`

Download and extract <a href="http://bochs.sourceforge.net/">bochs</a>  

`cd {directory of bochs}`

`./configure  --enable-debugger --enable-readline`

`./configure  --enable-gdb-stub --enable-readline`

`sudo make && sudo make install`

Reference: <a href="https://bochs.sourceforge.io/doc/docbook/user/compiling.html"> Compiling Bochs </a>

### Compile the OS
```sudo apt install -y gcc-9 nasm binutils grub-pc-bin xorriso mtools```

* Should only use `gcc-9` to compile myOS
* Install `mtools` if `grub-mkrescue` fails


### Run
```bochs -q```  


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

#### Enable Paging
To enable Paging, we need to do the following
* Setup page directory and page tables (at least for the kernel)
* Load page directory address to the cr3 register
* Set bit 32 (paging-enabling bit) of the cr0 register

Some tips:
* The kernel should be compiled starting at 0xc000 0000 but must be loaded at the lower part of the memory, we need to use linker script directives . and AT to do this (. is for indicating the compiling starting address; AT is for grub to know where in the mem to put the binary).
* At the very beginning, we should make an extra identity map of the lower part of the memory. After enabling paging, we should jump to the higer space and clear the identity map.

### Bootstrap
The memory management subsystem itself also needs memory, and it can't magically initialize itself -- it's like a chicken-egg (鸡生蛋，蛋生鸡) problem. To handle this, we need to write a very weak but simple memory allocator (like a boot_mem_allocator) for allocating memory for the strong but complex allocating system.


### Address Space

#### Higher Half
The x86 system supports an address space of 4-GiB; for reasons of compatibility and linker convention, the kernel space is mapped to the higer part of the virtual address space (above 0xc000 0000 in myOS); this is called a higher half kernel.



#### Linux Kernel Space
* This section is for Linux Memory Management

The kernel space is high 1G (0xc000 0000 ~ 0xffff ffff). Linux devides the physical memory into 3 Zones: DMA(low 16M), NORMAL(16 ~ 896M), HIGHMEM(896M ~).
* DMA zone is for hardwares
* NORMAL zone is direct mapped (physical addr = linear addr - page_offset)
    * I think the use for NORMAL zone is for high efficiency (Maybe)
* HIGHMEM zone is for dynamically mapping

## Thread and Process

Thread is the execution flow. Each thread has its own registers and stack. Every thread belongs to a certain process. Process provides its threads with resources. Thread is the unit of cpu scheduling; process is the unit of the OS resources allocation.

Mathematically, process = thread(s) + resources.