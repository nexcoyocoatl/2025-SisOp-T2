#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memory.h"
#include "instruction.h"
// #include "memory_linkedlist.h"

typedef uint8_t BYTE;

#define STRING_BUFFER 32
#define DEBUG 1

BYTE *memory_blocks;        // Memória física
BYTE *b_allocated_blocks;   // Indica se o bloco está alocado ou não (também é um byte, pra usar de boolean)

struct Instruction *instructions;
struct Process *processes;
size_t num_instructions;
size_t num_processes;

size_t count_lines_file(FILE *p_file);
size_t count_processes_from_file(FILE *p_file, size_t line_count);
int read_file(FILE *p_file);
int find_proc_by_name(char *name);
char *find_proc_by_id(size_t pid);

// ler de um .txt:
// IN(<nome do processo>,<espaco que ocupa>)
// OUT(<nome do processo>)

// exemplos:
// IN(A,8) requisita alocação de 8 espaços para o processo A
// IN(B,4)
// OUT(A) Libera todos os espaços do processo A
// OUT(B)

// A escolha do tipo de política a ser aplicada (worst ou circular fit) deverá ser realizada em tempo de execução pelo usuário

// se nao houver espaço suficiente para inserir processo:
// print "ESPAÇO INSUFICIENTE DE MEMORIA"
// continuar execução(?)

// ./main <txt> <memoria>
int main(int argc, char *argv[])
{
    // receber por args:
    // tamanho da memoria (Deverá ser assumido um tamanho sempre equivalente a uma potência de dois.)
    // nome do arquivo a ser aberto

    num_processes = 0;
    num_instructions = 0;

    size_t memory_size = 1024; // TODO: MUDAR PARA ESCOLHA DO USUÁRIO

    memory_blocks = malloc(sizeof(uint8_t) * memory_size);
    b_allocated_blocks = malloc(sizeof(uint8_t) * memory_size); 

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

    printf("%lu, %lu\n", line_count, proc_count);

    // Lê arquivo pela terceira vez (talvez mudar isso) e adiciona processos e instrucoes em suas respectivas listas
    read_file(p_file);    

    fclose(p_file);

    if (DEBUG)
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
    
    // perguntar politica (circular ou worst fit)
    //int option;
    //printf("Qual política de alocação você gostaria de implementar? \n");
    //printf("(1) Circular Fit \n (2) Worst Fit")
    // scanf("%d", &option);
    

    // TODO: será modificado pela lista de bytes acima
    struct Memory_block *block_list;

    initialize_block_list(&block_list, 2048);

    // for (size_t i = 0; i < 2048; i++)
    // {
    //     printf("%lu,", block_list[i].occupied);
    // }    
    // printf("\n");
    
    free(block_list);

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
            if ((pid = find_proc_by_name(proc_name)) == -1)
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
            if ((pid = find_proc_by_name(proc_name)) == -1)
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
                process_names[proc_count] = malloc(sizeof(char)*STRING_BUFFER);
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
int find_proc_by_name(char *name)
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

char *find_proc_by_id(size_t pid)
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

// TODO: Possivelmente serão adicionadas na linked list
// worst fit

// circular fit