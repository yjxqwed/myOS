# dirs
ROOT = ../..
USR_ROOT = ..

# build flags
G_PARAMS = -Wall -m32 -I$(USR_ROOT)/include -I$(ROOT)/include \
           -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore \
           -nostdinc -masm=intel -O3
LD_PARAMS = -m elf_i386

# ifeq ($(ver), release)
# G_PARAMS += -O3
# else
# G_PARAMS += -g -D KDEBUG -O0
# endif

# objects
SRC = $(USR_ROOT)/src

usr_lib_objs = $(SRC)/unistd.elf32 $(SRC)/stdio.elf32 $(SRC)/malloc.elf32 $(SRC)/string.elf32

lvsh_objs = lvsh.elf32

objects = $(usr_lib_objs) $(lvsh_objs)



# build rules
.PHONY = clean docker docker-release

all: lvsh

%.elf32: %.c
	gcc $(G_PARAMS) -o $@ -c $<

lvsh: $(USR_ROOT)/usr_linker.ld $(objects)
	ld $(LD_PARAMS) -T $< -o $@ $(objects)

clean:
	rm -rf lvsh $(objects)

docker:
	docker exec -w /mnt/external/myOS/usr/lvsh myos1 make

# docker-release:
# 	docker exec -w /mnt/external/myOS/usr/lvsh myos1 make ver=release

