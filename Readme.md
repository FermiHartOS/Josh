# Fermi Hart OS
Projeto de Microkernel simples com shell interativo.

## Compilação
Requisitos: `gcc-multilib`, `nasm`, `qemu-system-i386`.

1. Execute `make`
2. Rode `qemu-system-i386 -kernel myos.bin`

## Comandos do MiniBash
- `help`: Lista comandos
- `ver`: Versão do sistema
- `clear`: Limpa a tela
- `about`: Info sobre o projeto
- `halt`: Desliga a CPU
