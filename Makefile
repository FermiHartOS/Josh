# --- Configurações de Compilação ---
CC      = gcc
AS      = nasm
LD      = ld

# Flags de Compilação C
# -Werror: Warning é Erro. Pureza total.
CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra -Wpedantic -Werror \
          -fno-stack-protector -I include -nostdlib -fno-pie

# Flags de Assembly
ASFLAGS = -f elf32

# Flags do Linker
# --build-id=none: Remove o warning de build-id ignorado
# -n: Nmagic, desativa alinhamentos de página do Linux
LDFLAGS = -m32 -T linker.ld -nostdlib -z noexecstack -no-pie -n -Wl,--build-id=none
# --- Arquivos ---
BIN     = myos.bin
SRCDIR  = src
OBJDIR  = obj

OBJS    = $(OBJDIR)/boot.o \
          $(OBJDIR)/kernel.o \
          $(OBJDIR)/vga.o \
          $(OBJDIR)/keyboard.o \
          $(OBJDIR)/MiniBash.o \
          $(OBJDIR)/gdt.o

# --- Regras Principais ---

all: $(OBJDIR) $(BIN) verify
	@echo "--- [SUCESSO] Fermi Hart OS pronto para boot! ---"

# Linkagem e Verificação
$(BIN): $(OBJS)
	@echo "[LINK] $@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) -lgcc

# Nova Regra de Verificação Automática
verify:
	@echo "[CHECK] Verificando integridade Multiboot..."
	@if grub-file --is-x86-multiboot $(BIN); then \
		echo "  > Kernel validado com sucesso!"; \
	else \
		echo "  > [ERRO] Kernel inválido para Multiboot!"; \
		rm $(BIN); exit 1; \
	fi

# Compilação C
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compilação Assembly
$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	@echo "[AS] $<"
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

# --- Execução ---

run: all
	@echo "[QEMU] Iniciando Fermi Hart OS..."
	@qemu-system-i386 -kernel $(BIN) -machine type=pc-i440fx-3.1 -serial stdio

# Modo puramente via terminal (Serial)
run-cli: all
	@echo "[QEMU] Iniciando em modo Serial (sem janela)..."
	@qemu-system-i386 -kernel $(BIN) -nographic -serial mon:stdio

clean:
	@echo "[CLEAN] Limpando build..."
	@rm -rf $(OBJDIR) $(BIN)

.PHONY: all run run-cli clean verify
