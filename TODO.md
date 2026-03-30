1.2. Tratamento de Exceções
Atualmente só há handler para divisão por zero. Adicionar handlers genéricos para todas as exceções (0‑31) que imprimam o número e façam cli; hlt. Isso evita surpresas quando a CPU levantar uma exceção inesperada.

1.3. Keyboard: Suporte a Shift
O mapa de scancodes atualmente só gera letras minúsculas. Implementar um estado de shift (press/release) e converter para maiúsculas quando apropriado.

1.4. VGA: Correção no Backspace
No shell, quando o usuário pressiona backspace no início da linha, nada acontece (comportamento esperado). Mas o código atual em vga_put_char só apaga se cursor_x > 0. Isso está correto. Porém, se o cursor estiver na primeira coluna, ele não se move para a linha anterior. Para manter a UX, podemos ignorar – é aceitável.

1.5. Shell: Comando help Dinâmico
O help atual é uma lista estática. Podemos transformá-lo em uma função que itera sobre uma tabela de comandos, facilitando a adição de novos comandos sem alterar o help.

1.6. Makefile: Dependências Automáticas
Adicionar regras para gerar dependências (.d files) com -MMD do GCC, para que modificações em headers recompilam os arquivos corretos.

1.7. Debug Serial
Adicionar um driver serial mínimo (COM1) com funções serial_putchar e serial_puts. Usá-lo para logging em modo debug (via -d no QEMU). Isso ajuda a debugar exceções sem depender do VGA.

2. Preparação para a Fase 2: Gerenciamento de Memória
Antes de partir para alocadores e paging, precisamos de informações sobre a memória física e uma estrutura para gerenciá-la.

2.1. Obter o Mapa de Memória via Multiboot
O GRUB (ou QEMU) passa um ponteiro para a estrutura multiboot_info em EBX. Precisamos ler esse ponteiro no kernel_main e extrair a tabela de regiões de memória (mmap). Criar uma função que parseia essa tabela e armazena as regiões disponíveis.

2.2. Physical Memory Manager (PMM)
Implementar um bitmap de 4KB páginas. O bitmap pode ser alocado estáticamente (ex.: 128KB para 128MB de RAM). Funções:

pmm_init(mmap_info): marca páginas reservadas (kernel, VGA, etc.) como usadas.

pmm_alloc_page(): retorna endereço físico de uma página livre.

pmm_free_page(addr).

2.3. Simple Kernel Heap (kmalloc/kfree)
Com o PMM, podemos construir um alocador de pequenos blocos baseado em linked lists (ex.: buddy system ou slab). Inicialmente, uma implementação simples de heap usando uma lista de blocos livres (first-fit) será suficiente para alocar estruturas do kernel (GDT, IDT, futuros processos).

2.4. Paging (Pré‑requisito para multitasking)
Habilitar paging é o próximo passo lógico:

Criar um page directory e page tables para mapear os primeiros 4MB (identity mapping do kernel).

Usar o PMM para alocar as páginas das tabelas.

Em kernel.c, após o heap estar funcional, chamar paging_init().

Por que paging agora? Porque é a base para proteção entre kernel e userland, e também para mapear memória dinâmica. Além disso, permite mapear os 4GB de forma limpa.

3. Evolução do Shell para Usar o Heap
Com kmalloc e kfree, podemos melhorar o shell:

Histórico dinâmico (atualmente fixo em 16). Usar uma lista encadeada.

Buffer de linha de comando alocado dinamicamente (sem limite fixo de 128).

Comandos que geram saída podem ser armazenados em buffers alocados dinamicamente.

4. Preparação para Multitasking (Fase 4)
Multitasking exige:

TSS (Task State Segment) para ring transitions.

Context switch (salvar/restaurar registradores, alterar page directory).

Interrupção do timer como agendador.

4.1. TSS
Adicionar uma entrada na GDT para o TSS. Inicializar uma TSS com ss0 e esp0 apontando para uma pilha de kernel (para quando user mode levantar interrupção).

4.2. Estrutura de Processo
Definir struct process contendo:

pid

page_directory*

registers (EAX, EBX, etc., para salvar contexto)

stack (kernel stack)

state (RUNNING, READY, BLOCKED)

next (lista encadeada)

4.3. Agendador Round‑Robin
Modificar o handler do timer (irq_handler_timer) para, a cada tick, chamar schedule(). O schedule escolhe o próximo processo da lista de prontos e chama uma função assembly (switch_to) que salva contexto atual e restaura o próximo.

4.4. Syscalls
Criar uma entrada na IDT (ex.: int 0x80) que recebe o número da syscall em EAX e executa uma tabela de funções do kernel (ex.: sys_write, sys_exit). Isso exigirá uma transição de ring 3 para ring 0.

5. Melhorias na Experiência do Usuário (Sem perder a reatividade)
Comandos de ajuda com descrição (já parcial, mas pode ser estendido).

Variáveis de ambiente (ex.: PATH, HOME).

Comando cd (mudar diretório atual) – mesmo sem filesystem, podemos ter um conceito de "caminho atual" para simular.

Comando ps (listar processos) após multitasking.

Comando kill (enviar sinal para processo).

6. Documentação e Testes
Documentar a API do kernel (funções de alocação, syscalls, etc.) em um arquivo docs/API.md.

Adicionar um script de teste que executa uma sequência de comandos no shell e verifica a saída (usando QEMU com serial redirecionada).

Criar um diagrama de arquitetura mostrando as camadas (VGA, GDT, IDT, PMM, Paging, etc.).
