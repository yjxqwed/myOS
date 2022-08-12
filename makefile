# compile myos with gcc-9
GCC = gcc-9

# build flags
G_PARAMS = -Wall -m32 -I include -I include/lib -nostdlib -fno-builtin \
           -fno-exceptions -fno-leading-underscore -nostdinc -masm=intel
NASM_PARAMS = -I include -f elf32
LD_PARAMS = -m elf_i386

ifeq ($(ver), release)
G_PARAMS += -O3
else
G_PARAMS += -g -D KDEBUG -O0
endif

# objects
device_objs = device/kb.elf32 device/screen.elf32 device/pit.elf32 \
              device/tty.elf32 device/console.elf32 \
			  device/ide.elf32

kernel_asm_objs = kernel/asm/entry.elf32 \
                  kernel/asm/istub.elf32

kernel_c_objs = kernel/gdt.elf32 kernel/idt.elf32 \
                kernel/isr.elf32 kernel/kernel.elf32 \
                kernel/tss.elf32 kernel/init.elf32 \
                kernel/syscall.elf32 kernel/io.elf32

lib_objs = lib/debug.elf32 lib/utils.elf32 \
           lib/string.elf32 lib/kprintf.elf32 lib/bitmap.elf32 \
           lib/list.elf32

mm_objs = mm/kvmm.elf32 mm/pmem.elf32 mm/vmm.elf32

fs_objs = fs/myfs/fs.elf32 fs/myfs/file.elf32 fs/myfs/dir.elf32 \
          fs/myfs/inode.elf32 fs/myfs/utils.elf32

simplefs_objs = fs/simplefs/simplefs.elf32

arch_x86_objs = arch/x86/interrupt.elf32

thread_objs = thread/thread.elf32 thread/switch.elf32 thread/sync.elf32 \
              thread/process.elf32

kernel_test_objs = kernel/test/simplefs_test.elf32

objects = $(device_objs) $(kernel_asm_objs) $(kernel_c_objs) \
          $(lib_objs) $(mm_objs) $(arch_x86_objs) \
          $(thread_objs) $(simplefs_objs) $(kernel_test_objs)



# build rules
.PHONY = clean dump docker docker-release

all: mykernel.iso

%.elf32: %.c
	$(GCC) $(G_PARAMS) -o $@ -c $<

%.elf32: %.s
	nasm $(NASM_PARAMS) -o $@ $<

mykernel.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

clean:
	rm -rf *.o *.elf32 *.bin *.out iso *.iso $(objects)

dump:
	objdump -D -M intel mykernel.bin | less

docker:
	docker exec -w /mnt/external/myOS myos1 make

docker-release:
	docker exec -w /mnt/external/myOS myos1 make ver=release

mykernel.iso: mykernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp $< iso/boot/
	echo 'set timeout = 0' > iso/boot/grub/grub.cfg
	echo 'set default = 0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/mykernel.bin' >> iso/boot/grub/grub.cfg
	echo '  boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=$@ iso
	rm -rf iso
