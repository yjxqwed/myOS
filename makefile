G_PARAMS = -m32 -I include -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -nostdinc -masm=intel
NASM_PARAMS = -f elf32
LD_PARAMS = -m elf_i386

# objects = kernel/asm/loader.elf32 \
#           kernel/asm/isr.elf32 \
#           kernel.elf32 gdt.elf32 utils.elf32 screen.elf32 \
#           idt.elf32 system.elf32 debug.elf32 istub.elf32 isr.elf32 kb.elf32

driver_objs = driver/kb.elf32 driver/screen.elf32
kernel_asm_objs = kernel/asm/loader.elf32 \
                  kernel/asm/istub.elf32
kernel_c_objs = kernel/gdt.elf32 kernel/idt.elf32 \
                kernel/isr.elf32 kernel/kernel.elf32
lib_objs = lib/debug.elf32 lib/system.elf32 lib/utils.elf32 \
           lib/string.elf32

usr_asm_objs = usr/asm/test.elf32

# objects = drive.bin kernel_asm.bin kernel_c.bin lib.bin
objects = $(driver_objs) $(kernel_asm_objs) $(kernel_c_objs) $(lib_objs) $(usr_asm_objs)

.PHONY = clean

all: mykernel.bin

%.elf32: %.c
	gcc $(G_PARAMS) -o $@ -c $<

%.elf32: %.s
	nasm $(NASM_PARAMS) -o $@ $<

mykernel.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

install: mykernel.bin
	sudo cp $< /boot/mykernel.bin

clean:
	rm -rf *.o *.elf32 *.bin *.out iso *.iso *.img $(objects)

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
