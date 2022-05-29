arch ?= x86_64
kernel := build/kernel-$(arch).bin
user_binaries := build/user/
iso := build/os-$(arch).iso

linker_script := kernel/src/arch/$(arch)/linker.ld
user_linker_script := user/linker.ld
grub_cfg := kernel/src/arch/$(arch)/grub.cfg

extra_c_source_files := $(wildcard kernel/src/arch/$(arch)/c_code/extras/*.c)
extra_c_header_files := $(wildcard kernel/src/arch/$(arch)/c_code/extras/*.h)
extra_c_object_files := $(patsubst kernel/src/arch/$(arch)/c_code/extras/%.c, build/arch/$(arch)/c_code/extras/%.o, $(extra_c_source_files))

c_source_files := $(wildcard kernel/src/arch/$(arch)/c_code/*.c)
c_header_files := $(wildcard kernel/src/arch/$(arch)/c_code/*.h)
c_object_files := $(patsubst kernel/src/arch/$(arch)/c_code/%.c, build/arch/$(arch)/c_code/%.o, $(c_source_files))

shared_c_source_files := $(wildcard shared/*.c)
shared_c_header_files := $(wildcard shared/*.h)
shared_c_object_files := $(patsubst shared/%.c, build/arch/$(arch)/shared/%.o, $(shared_c_source_files))

user_c_source_files := $(wildcard user/*.c)
user_c_header_files := $(wildcard user/*.h)
user_c_object_files := $(patsubst user/%.c, build/arch/$(arch)/user/%.o, $(user_c_source_files))
user_c_elf_files := $(patsubst user/%.c, build/user/%.elf, $(user_c_source_files))

assembly_source_files := $(wildcard kernel/src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst kernel/src/arch/$(arch)/%.asm, build/arch/$(arch)/%.o, $(assembly_source_files))

.PHONY: all clean run iso

all: $(kernel)

clean:
	@sudo rm -r build
	@sudo umount /mnt/fatgrub
	@sudo losetup -d /dev/loop24
	@sudo losetup -d /dev/loop25

monitor: $(iso)
	@sudo qemu-system-x86_64 -s -d int,cpu_reset -drive format=raw,file=$(iso),if=ide,index=0 -monitor stdio

serial: $(iso)
	@sudo qemu-system-x86_64 -s -d int,cpu_reset -drive format=raw,file=$(iso),if=ide,index=0 -serial stdio

run: $(iso)
	@sudo qemu-system-x86_64 -s -drive format=raw,file=$(iso),if=ide,index=0 -serial stdio

#GDB: set arch i386:x86-64:intel, target remote localhost:1234, symbol-file build/kernel-x86_64.bin, bt
#GDB: `layout src` for nice view
#GDB: p var-name to print val
#GDB: set var-name=new-value

$(iso): $(kernel) $(grub_cfg) user
	@sudo dd if=/dev/zero of=$(iso) bs=512 count=32768
	@sudo parted $(iso) mklabel msdos
	@sudo parted $(iso) mkpart primary fat32 2048s 30720s
	@sudo parted $(iso) set 1 boot on
	@sudo losetup /dev/loop24 $(iso)
	@sudo losetup /dev/loop25 $(iso) -o 1048576
	@sudo mkdosfs -F32 -f 2 /dev/loop25
	@sudo mount /dev/loop25 /mnt/fatgrub
	@sudo grub-install --root-directory=/mnt/fatgrub --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop24
	@sudo mkdir -p /mnt/fatgrub/boot/grub
	@sudo cp $(kernel) /mnt/fatgrub/boot/kernel.bin
	@sudo cp $(grub_cfg) /mnt/fatgrub/boot/grub/
	@sudo cp -R $(user_binaries) /mnt/fatgrub/boot/
	@sudo cp war_and_peace.txt /mnt/fatgrub/boot/war_and_peace.txt
	@sudo umount /mnt/fatgrub
	@sudo losetup -d /dev/loop24
	@sudo losetup -d /dev/loop25

$(kernel): $(assembly_object_files) $(linker_script) $(c_object_files) $(c_header_files) $(extra_c_object_files) $(extra_c_header_files)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files) $(extra_c_object_files)

user: $(user_c_elf_files)

$(user_binaries)%.elf: build/arch/$(arch)/user/%.o $(shared_c_object_files)
	@mkdir -p $(shell dirname $@)
	@ld -n -T $(user_linker_script) -o $@ $^

build/arch/$(arch)/%.o: kernel/src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -f elf64 -F dwarf -g $< -o $@

build/arch/$(arch)/c_code/%.o: kernel/src/arch/$(arch)/c_code/%.c
	@mkdir -p $(shell dirname $@)
	gcc -c -fno-stack-protector -g $< -o $@

build/arch/$(arch)/c_code/extras/%.o: kernel/src/arch/$(arch)/c_code/extras/%.c
	@mkdir -p $(shell dirname $@)
	gcc -c -fno-stack-protector -g $< -o $@

build/arch/$(arch)/user/%.o: user/%.c 
	@mkdir -p $(shell dirname $@)
	gcc -c -fno-builtin -fno-stack-protector -g $< -o $@

build/arch/$(arch)/shared/%.o: shared/%.c
	@mkdir -p $(shell dirname $@)
	gcc -c -fno-builtin -fno-stack-protector -g $< -o $@
