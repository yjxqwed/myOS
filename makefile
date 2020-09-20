GPP_PARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore
NASM_PARAMS = -f elf32
LD_PARAMS = -m elf_i386

objects = loader.elf32 kernel.elf32 gdt.elf32

.PHONY = clean

# kernel.s: kernel.cpp:
# 	g++ $(GPP_PARAMS) kernel.cpp -s -o kernel.s

%.elf32: %.cpp
	g++ $(GPP_PARAMS) -o $@ -c $<

%.elf32: %.s
	nasm $(NASM_PARAMS) -o $@ $<

mykernel.bin: linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

install: mykernel.bin
	sudo cp $< /boot/mykernel.bin

# .PHONY clean
clean:
	rm -rf *.o *.elf32 *.bin *.out iso *.iso *.img

run:
	bochs -f bochsrc

mnt:
	sudo mount myOS.img /mnt/myOS/

umnt:
	sudo umount /mnt/myOS/

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


boot.img:
	-nasm boot.s -o boot.bin
	-dd if=/dev/zero of=a.img bs=512 count=2880
	-dd if=boot.bin of=boot.img bs=512 count=1
	-dd if=a.img of=boot.img bs=512 seek=1 count=2879 bs=512