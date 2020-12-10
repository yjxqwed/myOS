# build flags
G_PARAMS = -Wall -m32 -I include -I include/lib -nostdlib -fno-builtin \
           -fno-exceptions -fno-leading-underscore -nostdinc -masm=intel
NASM_PARAMS = -I include -f elf32
LD_PARAMS = -m elf_i386

ifeq ($(ver), release)
G_PARAMS += -O3
else
G_PARAMS += -D KDEBUG
endif

# objects
device_objs = device/kb.elf32 device/screen.elf32 device/pit.elf32 \
              device/hd.elf32 device/console.elf32
kernel_asm_objs = kernel/asm/entry.elf32 \
                  kernel/asm/istub.elf32
kernel_c_objs = kernel/gdt.elf32 kernel/idt.elf32 \
                kernel/isr.elf32 kernel/kernel.elf32 \
                kernel/tss.elf32 kernel/proc.elf32 \
                kernel/init.elf32
lib_objs = lib/debug.elf32 lib/utils.elf32 \
           lib/string.elf32 lib/kprintf.elf32 lib/bitmap.elf32 \
           lib/list.elf32

mm_objs = mm/kvmm.elf32 mm/pmem.elf32

usr_asm_objs = usr/asm/test.elf32

arch_x86_objs = arch/x86/interrupt.elf32

thread_objs = thread/thread.elf32 thread/switch.elf32 thread/sync.elf32

objects = $(device_objs) $(kernel_asm_objs) $(kernel_c_objs) \
          $(lib_objs) $(usr_asm_objs) $(mm_objs) $(arch_x86_objs) \
          $(thread_objs)



# build rules
.PHONY = clean dump

all: mykernel.iso

%.elf32: %.c
	gcc $(G_PARAMS) -o $@ -c $<

%.elf32: %.s
	nasm $(NASM_PARAMS) -o $@ $<

mykernel.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

# install: mykernel.bin
# 	sudo cp $< /boot/mykernel.bin

clean:
	rm -rf *.o *.elf32 *.bin *.out iso *.iso $(objects)

dump:
	objdump -D -M intel mykernel.bin | less

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
