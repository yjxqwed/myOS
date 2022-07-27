# myOS

I use many operating systems all day but can't clearly understand how they work and are implemented as a whole. This project is to write a very simple kernel so that I can figure out the key concepts and principles of an OS.

## Key Features
* Multi-processing
* Paging system (virtual memory management)
* A file system
* Different types of syscalls
* Interrupts handling
* etc.

By Jiaxing Yang

From 2020.9

# References
* <a href="https://pdos.csail.mit.edu/6.828/2018/"> MIT 6.828 (2018) </a>
* <a href="https://wiki.osdev.org"> osdev.org </a>
* <a href="http://www.osdever.net/bkerndev/Docs/title.htm"> bkerndev </a>
* <a href="http://skelix.net/skelixos/index_en.html"> skelix os </a>
* <a href="https://www.gnu.org/software/grub/manual/multiboot/multiboot.html"> GNU multiboot </a>
* <a href="http://littleosbook.github.io/#linking-the-kernel"> The little book about OS development </a>
* <a href="https://book.douban.com/subject/3735649/"> 《Orange'S:一个操作系统的实现》 于渊 </a>
* <a href="https://book.douban.com/subject/26745156/"> 《操作系统真象还原》 郑钢 </a>
* <a href=""> 《Linux 内核源代码情景分析》 毛德操，胡希明 </a>

# Screenshots
### When myOS boots
![boot](/screenshots/boot.png)
### A simple shell `lvsh`
![lvsh](/screenshots/lvsh.png)
* lvsh has some builtin commands: ps, ls, mm, etc.
* Can also execute a binary form it and pass command line args to the process.

### A simple software `cat` (yes, it is the cat on Linux XD)
![cat](/screenshots/cat.png)
### A software `nothing` used to test
![nothing_n](/screenshots/nothing_normal.png)
### Use `nothing` to trigger a page fault
![nothing_pf1](/screenshots/nothing_pagefault1.png)
![nothing_pf2](/screenshots/nothing_pagefault2.png)
