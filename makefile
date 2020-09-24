GPP_PARAMS = -m32 -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -masm=intel
NASM_PARAMS = -f elf32
LD_PARAMS = -m elf_i386

objects = loader.elf32 kernel.elf32 gdt.elf32 utils.elf32 screen.elf32 \
          idt.elf32 system.elf32 debug.elf32 istub.elf32 isr.elf32

.PHONY = clean

all: mykernel.bin

%.elf32: %.c
	gcc $(GPP_PARAMS) -o $@ -c $<

%.elf32: %.s
	nasm $(NASM_PARAMS) -o $@ $<

mykernel.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

install: mykernel.bin
	sudo cp $< /boot/mykernel.bin

clean:
	rm -rf *.o *.elf32 *.bin *.out iso *.iso *.img

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