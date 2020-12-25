# myOS

By Jiaxing Yang

From 2020.9

# References
* <a href="https://pdos.csail.mit.edu/6.828/2018/"> MIT 6.828 (2018) </a>
* <a href="wiki.osdev.org"> osdev.org </a>
* <a href="http://www.osdever.net/bkerndev/Docs/title.htm"> bkerndev </a>
* <a href="http://skelix.net/skelixos/index_en.html"> skelix os </a>
* <a href="https://www.gnu.org/software/grub/manual/multiboot/multiboot.html"> GNU multiboot </a>
* <a href="http://littleosbook.github.io/#linking-the-kernel"> The little book about OS development </a>
* <a href="https://book.douban.com/subject/3735649/"> 《Orange'S:一个操作系统的实现》 于渊 </a>
* <a href="https://book.douban.com/subject/26745156/"> 《操作系统真象还原》 郑钢 </a>
* <a href=""> 《Linux 内核源代码情景分析》 毛德操，胡希明 </a>


# Known Bugs
1. ~~`kfree` seems buggy. When calling it concurrently, sometimes (very unlikely) there will be a strange page fault exception. page fault addr = `0x28`~~ FIXED
