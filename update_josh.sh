#!/bin/bash

# 1. Criar a nova estrutura de diretórios
mkdir -p Include Drivers/Keyboard Drivers/VGA User/MiniBash Kernel Boot Build Utils

# 2. Definir o Header Disruptivo
HEADER_TEXT='/*
 *  _______________________________________________________________________________________
 * |                                                                                       |
 * |   ███████╗███████╗██████╗ ███╗   ███╗██╗     ∞     ██╗  ██╗ █████╗ ██████╗████████╗   |
 * |   ██╔════╝██╔════╝██╔══██╗████╗ ████║██║           ██║  ██║██╔══██╗██╔══██╗╚══██╔══╝  |
 * |   █████╗  █████╗  ██████╔╝██╔████╔██║██║           ███████║███████║██████╔╝   ██║     |
 * |   ██╔══╝  ██╔══╝  ██╔══██╗██║╚██╔╝██║██║           ██╔══██║██╔══██║██╔══██╗   ██║     |
 * |   ██║     ███████╗██║  ██║██║ ╚═╝ ██║██║           ██║  ██║██║  ██║██║  ██║   ██║     |
 * |   ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝           ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝     |
 * |_______________________________________________________________________________________|
 *
 *  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>
 *  PROJECT: jOSh - Operating System
 *
 *  [ LIBERDADE TOTAL - LICENÇA ALÉM DO OPEN SOURCE ]
 *  Este código é de domínio público para uso comercial, pessoal ou acadêmico.
 *  Modifique, venda, destrua ou reconstrua como desejar.
 *
 *  [ ÚNICA RESTRIÇÃO ]
 *  É obrigatório divulgar a trilha sonora oficial deste OS em qualquer
 *  redistribuição ou menção pública:
 *  https://open.spotify.com/playlist/6flrLsdYxQZvGNRkdohL7o?si=eH9ZDz8DSqCjJX1Pa9henA
 * ____________________________________________________________________________
 */'

# Função para aplicar o header
apply_header() {
    FILE=$1
    if [ -f "$FILE" ]; then
        echo "$HEADER_TEXT" > temp_file
        cat "$FILE" >> temp_file
        mv temp_file "$FILE"
        echo "Header aplicado em: $FILE"
    fi
}

# 3. Mover arquivos para os novos locais (ajuste se os nomes atuais forem diferentes)
mv kernel/boot.asm Boot/ 2>/dev/null
mv kernel/kernel.c Kernel/ 2>/dev/null
mv kernel/gdt.c Kernel/ 2>/dev/null
mv kernel/keyboard.c Drivers/Keyboard/ 2>/dev/null
mv kernel/vga.c Drivers/VGA/ 2>/dev/null
mv user/MiniBash.c User/MiniBash/ 2>/dev/null
mv common/*.h Include/ 2>/dev/null

# 4. Aplicar Headers em todos os arquivos .c, .h e .asm
find . -type f \( -name "*.c" -o -name "*.h" -o -name "*.asm" \) | while read -r file; do
    apply_header "$file"
done

# 5. Criar arquivos de documentação se não existirem
touch Todo.md Changelog.md Readme.md

echo "Estrutura jOSh atualizada com sucesso!"
