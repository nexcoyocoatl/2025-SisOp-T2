#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruction.h"
#include "memory_linkedlist.h"
#include "memory_tree.h"

typedef uint8_t BYTE;

#define STRING_BUFFER 32
#define DEBUG 1

BYTE *memory_blocks;        // Memória física
uint8_t *b_allocated_blocks;   // Indica se o bloco está alocado ou não (também é um uint8_t, pra usar de boolean)

struct Instruction *instructions;
struct Process *processes;
size_t num_instructions;
size_t num_processes;

size_t count_lines_file(FILE *p_file);
size_t count_processes_from_file(FILE *p_file, size_t line_count);
int read_file(FILE *p_file);
int find_proc_id_by_name(char *name);
char *find_proc_name_by_id(size_t pid);
int find_proc_size_by_id(size_t pid);
void print_memory_blocks(BYTE *b_allocated_blocks, size_t memory_size);
void print_memory_bytes(BYTE *memory_list, size_t memory_size);

enum {
    CIRCULAR = 1,
    WORST,
    BUDDY
};

// ler de um .txt:
// IN(<nome do processo>,<espaco que ocupa>)
// OUT(<nome do processo>)

// A escolha do tipo de política a ser aplicada (worst ou circular fit) deverá ser realizada em tempo de execução pelo usuário

// ./main <txt> <tamanho_memoria>
int main(int argc, char *argv[])
{
    // receber por args:
    // tamanho da memoria (Deverá ser assumido um tamanho sempre equivalente a uma potência de dois.)
    // nome do arquivo a ser aberto

    num_processes = 0;
    num_instructions = 0;

    srand(time(NULL));

    size_t memory_size = 128;    // TODO: MUDAR PARA ESCOLHA DO USUÁRIO
    uint8_t strategy = CIRCULAR;   // TODO: MUDAR PARA ESCOLHA DO USUÁRIO

    if ( memory_size > 0 && ((memory_size & (memory_size - 1)) != 0) ) { return 1; } // checa se é >0 e potencia de 2

    memory_blocks = malloc(sizeof(uint8_t) * memory_size);
    b_allocated_blocks = malloc(sizeof(uint8_t) * memory_size);

    // Zera memória e os booleanos de blocos allocados
    for(size_t i = 0; i < memory_size; i++)
    {
        
        b_allocated_blocks[i] = DISALLOC;
        memory_blocks[i] = 0;
        // memory_blocks[i] = rand() % rand() % 256; // Exemplo de garbage aleatório por toda memória
    }

    FILE *p_file;
    
    // TODO: por enquanto só recebe nome do arquivo (ex*.txt), depois vai aceitar tamanho de memória e talvez a política
    if (argc == 2)
    {
        p_file = fopen(argv[1], "r");
    }
    else
    {
        printf("usage: sisop_t2 <parameter>\n");
        char input[100] = "";
        printf("Type the name of the file: ");
        scanf("%s", input);
        printf("\n");
        p_file = fopen(input, "r");
    }
    
    if (p_file == NULL)
    {
        printf("File not found.\nExiting program...\n");
        return 1;
    }

    // Conta linhas de instrucoes
    size_t line_count = count_lines_file(p_file);

    if (line_count == 0)
    {
        printf("Empty file.\nExiting program...\n");
        return 1;
    }

    instructions = malloc(sizeof(struct Instruction) * line_count);

    // Conta número de processos individuais
    size_t proc_count = count_processes_from_file(p_file, line_count);

    processes = malloc(sizeof(struct Process) * proc_count);

    struct Memory_list* memory_list;
    struct Memory_tree* memory_tree;

    if (strategy == BUDDY)
    {
        memory_tree = memtree_create(memory_size);
    }
    else
    {
        memory_list = memlist_create(memory_size);
    }

    // Lê arquivo pela terceira vez (talvez mudar isso) e adiciona processos e instrucoes em suas respectivas listas
    read_file(p_file);

    fclose(p_file);    

    // perguntar politica (circular ou worst fit)
    //int option;
    //printf("Qual política de alocação você gostaria de implementar? \n");
    //printf("(1) Circular Fit \n (2) Worst Fit")
    // scanf("%d", &option);

    // Mostra a lista de processos e instruções
    if (DEBUG > 2)
    {
        for (size_t i = 0; i < num_processes; i++)
        {
            printf("proc_name: %s, size: %lu\n", processes[i].name, processes[i].size);
        }

        for (size_t i = 0; i < num_instructions; i++)
        {
            printf("instruction_type: %d, pid: %lu\n", instructions[i].operation, instructions[i].pid);
        }
    }

    if (strategy == BUDDY)
    {
        memtree_print(memory_tree);
    }
    else
    {
        memlist_print(memory_list);
    }

    if (DEBUG)
    {
        print_memory_blocks(b_allocated_blocks, memory_size);
        print_memory_bytes(memory_blocks, memory_size);
    }
    printf("\n");

    // Executa instruções
    for (size_t i = 0; i < num_instructions; i++)
    {
        size_t proc_start_address;
        size_t proc_size = find_proc_size_by_id(instructions[i].pid);
        char *proc_name = find_proc_name_by_id(instructions[i].pid);

        // IN(proc, size)
        if (instructions[i].operation == ALLOC)
        {
            // Estratégias de alocação
            switch (strategy)
            {
            case CIRCULAR:
                proc_start_address = (size_t)(memlist_add_circular(memory_list, instructions[i].pid, proc_size));
                break;
            case WORST:
                proc_start_address = (size_t)memlist_add_worst(memory_list, instructions[i].pid, proc_size);
                break;
            case BUDDY:
                proc_start_address = (size_t)memtree_add_buddy(memory_tree, instructions[i].pid, proc_size);
                break;
            }
            
            if (proc_start_address == -1)
            {
                printf("PROCESSO %s: TAMANHO %lu, NÃO ALOCADO. ESPAÇO INSUFICIENTE DE MEMÓRIA\n", proc_name, proc_size);
            }
            else
            {
                for (int j = proc_start_address; j < proc_start_address + proc_size; j++)
                {
                    memory_blocks[j] = rand() % 256;
                    b_allocated_blocks[j] = ALLOC;
                }
                printf("PROCESSO %s: TAMANHO %lu, INSERIDO NO ENDEREÇO 0x%07zX (%lu)\n",
                        proc_name, proc_size, proc_start_address, proc_start_address);
            }
        }
        else    // OUT(proc)
        {
            switch (strategy)
            {
            case BUDDY:
                proc_start_address = (size_t)(memtree_remove_node(memory_tree, instructions[i].pid));
                break;
            default:
                proc_start_address = (size_t)(memlist_remove_node(memory_list, instructions[i].pid));
                break;
            }

            if (proc_start_address == -1)
            {
                printf("PROCESSO %s: NÃO ENCONTRADO PARA REMOÇÃO\n", proc_name);
            }
            else
            {
                for (int j = proc_start_address; j < proc_start_address + proc_size; j++)
                {
                    b_allocated_blocks[j] = DISALLOC;
                }
                printf("PROCESSO %s: TAMANHO %lu, REMOVIDO DO ENDEREÇO 0x%07zX (%lu)\n",
                       proc_name, proc_size, proc_start_address, proc_start_address);
            }
        }

        if (strategy == BUDDY)
        {
            memtree_print(memory_tree);
        }
        else
        {
            memlist_print(memory_list);
        }

        if (DEBUG)
        {
            print_memory_blocks(b_allocated_blocks, memory_size);
            print_memory_bytes(memory_blocks, memory_size);
        }

        if (DEBUG > 1)
        {
            if (strategy == BUDDY)
            {
                memtree_dump(memory_tree);
            }
            else
            {
                memlist_dump(memory_list);
            }
        }

        printf("\n");
    }

    

    if (strategy == BUDDY)
    {
        // Testes pro clear (retirar depois)
        // memtree_add_buddy(memory_tree, 0, 2);
        // memtree_add_buddy(memory_tree, 1, 2);
        // memtree_remove_node(memory_tree, 0);
        // memtree_add_buddy(memory_tree, 2, 2);
        memtree_clear(memory_tree);
        // memtree_dump(memory_tree);
    }
    else
    {
        // Testes pro clear (retirar depois)
        // memlist_flush(memory_list, memory_size);
        // memlist_add_circular(memory_list, 0, 2);
        // memlist_add_circular(memory_list, 1, 2);
        // memlist_remove_node(memory_list, 0);
        // memlist_add_worst(memory_list, 2, 2);
        memlist_clear(memory_list);
        // memlist_dump(memory_list);
    }

    // "Desaloca" toda memória mudando todos booleans para desalocados
    // Memória física continua com o que tinha nos blocos, que agora é garbage
    for (size_t i = 0; i < memory_size; i++)
    {
        b_allocated_blocks[i] = DISALLOC;
    }

    return 0;
}

// Lê o arquivo .txt
int read_file(FILE *p_file)
{
    char s[STRING_BUFFER];
    char proc_name[STRING_BUFFER];
    int size;
    
    while(fgets(s, sizeof(s), p_file))
    {
        size_t pid;
        for (size_t i = 0; i < STRING_BUFFER; i++)
        {
            proc_name[i] = ' ';
        }
        proc_name[STRING_BUFFER-1] = '\0';
        size = 0;

        // Um tipo de regex do C pra ser usado com IN(nome,num)
        if (sscanf(s, "%*[^'(']%*c%[^',']%*c%d%*[^'\0']", proc_name, &size) == 2)
        {
            if ((pid = find_proc_id_by_name(proc_name)) == -1)
            {
                pid = num_processes;
                strcpy(processes[pid].name, proc_name);
                processes[pid].pid = pid;                
                num_processes++;
            }
            processes[pid].size = (size_t)size;

            instructions[num_instructions].operation = ALLOC;
            instructions[num_instructions].pid = pid;
            num_instructions++;

            continue;
        }
        // Um tipo de regex do C pra ser usado com OUT(nome)
        else if (sscanf(s, "%*[^'(']%*c%[^')']%*[^'\0']", proc_name) == 1)
        {
            if ((pid = find_proc_id_by_name(proc_name)) == -1)
            {
                printf("Invalid instruction.\nExiting program...\n");
                return 0;
            }

            instructions[num_instructions].operation = DISALLOC;
            instructions[num_instructions].pid = pid;
            num_instructions++;

            continue;
        }

        printf("Invalid instruction.\nExiting program...\n");
        return 0;
    }

    return 1;
}

// Conta número de linhas de instrucoes
size_t count_lines_file(FILE *p_file)
{
    size_t line_count = 0;
    size_t char_count;
    char c;

    fseek(p_file, 0, SEEK_SET);

    while(!feof(p_file))
    {
        c = fgetc(p_file);

        if (c == '\n')
        {
            if (char_count > 1) // Apenas se existe algo na linha
            {
                line_count++;
            }

            char_count = 0;
            continue;
        }

        if (c == EOF && char_count > 0) // Caso não tenha '/n' na última linha
        {
            line_count++;
            break;
        }
        
        char_count++;
    }

    fseek(p_file, 0, SEEK_SET);

    return line_count;
}

// Conta numero de processos individuais do .txt
size_t count_processes_from_file(FILE *p_file, size_t line_count)
{
    size_t proc_count = 0;
    size_t char_count = 0;
    char proc_name[STRING_BUFFER];
    char c;
    char **process_names;

    process_names = malloc(sizeof(char *) * line_count);

    fseek(p_file, 0, SEEK_SET);

    while(!feof(p_file))
    {
        c = fgetc(p_file);

        if (c == '(')
        {
            int duplicate_process = 0;
            char_count = 0;
            
            while (((c = fgetc(p_file)) != ',' || c != ')') && isalpha(c))
            {
                proc_name[char_count] = c;
                char_count++;
            }
            proc_name[char_count] = '\0';

            for (size_t i = 0; i < proc_count; i++)
            {
                if (strcmp(proc_name, process_names[i]) == 0)
                {
                    duplicate_process = 1;
                    break;
                }
            }

            if (duplicate_process)
            {
                continue;
            }

            if (char_count > 0)
            {
                process_names[proc_count] = malloc(sizeof(char) * STRING_BUFFER);
                strncpy(process_names[proc_count], proc_name, STRING_BUFFER);
                proc_count++;
            }
        }
    }

    fseek(p_file, 0, SEEK_SET);

    for (size_t i = 0; i < proc_count; i++)
    {
        free(process_names[i]);
    }
    free(process_names);

    return proc_count;
}

// Funcoes auxiliares pra achar processo por nome ou pid
int find_proc_id_by_name(char *name)
{
    if (num_processes == 0)
    {
        return -1;
    }

    for (size_t i = 0; i < num_processes; i++)
    {
        if (strcmp(name, processes[i].name) == 0)
        {
            return processes[i].pid;
        }
    }

    return -1;
}

char *find_proc_name_by_id(size_t pid)
{
    if (num_processes == 0)
    {
        return "";
    }

    for (size_t i = 0; i < num_processes; i++)
    {
        if (pid == processes[i].pid)
        {
            return processes[i].name;
        }
    }

    return "";
}

int find_proc_size_by_id(size_t pid)
{
    if (num_processes == 0)
    {
        return 0;
    }

    for (size_t i = 0; i < num_processes; i++)
    {
        if (pid == processes[i].pid)
        {
            return processes[i].size;
        }
    }

    return 0;
}

// Imprime blocos de memória
void print_memory_blocks(BYTE *b_allocated_blocks, size_t memory_size)
{
    for(size_t i = 0; i < memory_size; i++)
    {
        if (i%64 == 0) // Cada 64 bytes (cada byte sendo 00-FF, 256 bits)
        {
            printf("0x%07zX ", i * 256);  // Cada fila de Hex vai de 0x0000 até 0x4000 (0-16383 bits, 64 bytes)
        }

        if (b_allocated_blocks[i] == ALLOC)
        {
            printf("█");
        }
        else
        {
            printf("░");
        }

        if (i%64 == 63 && i != memory_size-1)
        {
            printf("\n");
        }
    }
    printf("\n");
}

// Imprime memória byte por byte em hexadecimal, como um hexdump de um programa
void print_memory_bytes(BYTE *memory_blocks, size_t memory_size)
{
    for(size_t i = 0; i < memory_size; i++)
    {
        if (i%32 == 0) // Cada 32 bytes (cada byte sendo 00-FF, 256 bits)
        {
            printf("0x%07zX", i * 256);  // Cada fila de Hex vai de 0x0000 até 0x2000 (0-8192 bits, 32 bytes)
        }

        printf(" %02hhX", memory_blocks[i]);

        if (i%32 == 31 && i != memory_size-1)
        {
            printf("\n");
        }
    }
    printf("\n");
}