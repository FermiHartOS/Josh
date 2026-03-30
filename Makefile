# --- F E R M I  ∞  H A R T  OS [jOSh] ---
CC      = gcc
AS      = nasm
IMG     = build/jOSh.img

CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra -Wpedantic -Werror \
          -I include -fno-stack-protector -fno-pie -nostdlib

LDFLAGS = -m32 -T Linker.ld -nostdlib -z noexecstack -no-pie -n -Wl,--build-id=none

OBJS = build/boot.o \
       build/kernel.o \
       build/gdt.o \
       build/timer.o \
       build/pmm.o \
       build/kmalloc.o \
       build/vga.o \
       build/keyboard.o \
       build/splash.o \
       build/minibash.o \
       build/idt.o \
       build/idt_asm.o \
       build/nosound.o

all: build_dir $(IMG) verify

$(IMG): $(OBJS)
	@echo "[LINK] $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) -lgcc

build/boot.o: boot/boot.asm
	@echo "[AS] $<"
	@$(AS) -f elf32 $< -o $@

build/kernel.o: kernel/kernel.c
	@echo "[KERN] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/gdt.o: kernel/gdt.c
	@echo "[KERN] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/timer.o: kernel/timer.c
	@echo "[KERN] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/pmm.o: kernel/pmm.c
	@echo "[KERN] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/kmalloc.o: kernel/kmalloc.c
	@echo "[KERN] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/vga.o: drivers/vga/vga.c
	@echo "[DRV] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: drivers/keyboard/keyboard.c
	@echo "[DRV] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/splash.o: drivers/splash/splash.c
	@echo "[DRV] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/minibash.o: user/minibash/minibash.c
	@echo "[APP] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: kernel/idt.c
	@echo "[KERN] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/idt_asm.o: kernel/idt.asm
	@echo "[AS] $<"
	@$(AS) -f elf32 $< -o $@

build/nosound.o: drivers/nosound/nosound.c
	@echo "[DRV] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build_dir:
	@mkdir -p build

verify:
	@echo "[CHECK] Verificando Integridade..."
	@grub-file --is-x86-multiboot $(IMG) && echo "  > jOSh Kernel VALIDADO" || exit 1

run: all
	@echo "[QEMU] Iniciando jOSh OS..."
	@qemu-system-i386 -kernel build/jOSh.img -machine type=pc-i440fx-3.1 -serial mon:stdio

nosound: all
	@echo "[QEMU] Iniciando jOSh OS (WSL2 Audio Enabled)..."
	@qemu-system-i386 \
		-kernel build/jOSh.img \
		-machine type=pc-i440fx-3.1,pcspk-audiodev=snd0 \
		-audiodev pa,id=snd0 \
		-serial mon:stdio

clean:
	@echo "[CLEAN] Removendo build/..."
	@rm -rf build/
