#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "instruction.h"
#include "memory_linkedlist.h"
#include "memory_tree.h"

typedef uint8_t BYTE;

#define STRING_BUFFER 32
#define DEBUG 0

BYTE *memory_blocks;        // Memória física
uint8_t *allocated_blocks;  // Indica se o bloco está alocado ou não (também é um uint8_t, pra usar de boolean)

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
void print_memory_blocks(BYTE *allocated_blocks, size_t memory_size);
void print_memory_bytes(BYTE *memory_list, size_t memory_size);

enum {
    CIRCULAR = 1,
    WORST,
    BUDDY
};

int main(int argc, char *argv[])
{
    memory_blocks = NULL;
    allocated_blocks = NULL;
    instructions = NULL;
    processes = NULL;

    num_processes = 0;
    num_instructions = 0;
    size_t line_count = 0;
    size_t proc_count = 0;

    int int_memory_size = 0;
    size_t memory_size = 0;
    int8_t strategy = -1;

    FILE *p_file = NULL;
    struct Memory_list* memory_list = NULL;
    struct Memory_tree* memory_tree = NULL;

    #ifdef _WIN32
    uint16_t old_page = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
    #endif

    uint8_t dbg = DEBUG;

    srand(time(NULL));

    // Zera memória e os booleanos de blocos allocados
    for(size_t i = 0; i < memory_size; i++)
    {
        allocated_blocks[i] = DISALLOC;
        memory_blocks[i] = 0;
    }

    // Lê parâmetros da linha de comando
    for (size_t i = 0; i < argc; i++)
    {
        if (strcmp(argv[i],"-i") == 0 && argc > i+1)
        {
            p_file = fopen(argv[i+1], "r");
        }

        if (strcmp(argv[i],"-m") == 0 && argc > i+1)
        {
            int_memory_size = atoi(argv[i+1]);
        }
        
        if (strcmp(argv[i],"-s") == 0 && argc > i+1)
        {
            char *s = argv[i+1];
            for (size_t j = 0; s[j] != '\0'; j++)
            {
                s[j] = toupper(s[j]);
            }

            if (strcmp(s,"C") == 0 || strcmp(s,"CIRCULAR") == 0)
                { strategy = CIRCULAR; }
            else if (strcmp(s,"W") == 0 || strcmp(s,"WORST") == 0)
                { strategy = WORST; }
            else if (strcmp(s,"B") == 0 || strcmp(s,"BUDDY") == 0)
                { strategy = BUDDY; }
            else
            {
                strategy = -1;
            }
        }

        if (strcmp(argv[i],"-d") == 0 && argc > i+1)
        {
            dbg = atoi(argv[i+1]);
        }
    }

    // Caso os parâmetros não tenham sido inseridos
    // Verifica se arquivo é válido
    while (p_file == NULL)
    {
        char input[100] = "";
        printf("Arquivo não encontrado\n");
        printf("Digite o nome do arquivo de instruções: ");
        scanf("%s", input);
        printf("\n");
        p_file = fopen(input, "r");
    }

    // Verifica se memória é > 0 e potencia de 2
    while ( int_memory_size <= 0 || ((int_memory_size & (int_memory_size - 1)) != 0) )
    {
        char input[100] = "";
        
        if (int_memory_size <=0)
        {
            printf("Memória não pode ser menor ou igual a 0.\n");
        }
        else
        {
            printf("Memória não é potência de 2.\n");
        }
        printf("Digite o valor do tamanho da memória: ");
        scanf("%s", input);
        printf("\n");
        int_memory_size = atoi(input);
    }
    
    // Verifica se estratégia é válida
    while (strategy <= -1)
    {
        char input[100] = "";
        printf("Estratégia não encontrada.\n");
        printf("Digite o nome da estratégia: ");
        scanf("%s", input);
        printf("\n");

        char *s = input;
        for (size_t i = 0; s[i] != '\0'; i++)
        {
            s[i] = toupper(s[i]);
        }

        if (strcmp(s,"C") == 0 || strcmp(s,"CIRCULAR") == 0)
            { strategy = CIRCULAR; }
        else if (strcmp(s,"W") == 0 || strcmp(s,"WORST") == 0)
            { strategy = WORST; }
        else if (strcmp(s,"B") == 0 || strcmp(s,"BUDDY") == 0)
            { strategy = BUDDY; }
        else
        {
            strategy == -1;
        }
    }

    memory_size = (size_t)int_memory_size;
    memory_blocks = malloc(sizeof(uint8_t) * memory_size);
    allocated_blocks = malloc(sizeof(uint8_t) * memory_size);

    // Conta linhas de instruções
    line_count = count_lines_file(p_file);

    if (line_count == 0)
    {
        printf("Arquivo vazio.\nEncerrando programa...\n");
        return 1;
    }

    instructions = malloc(sizeof(struct Instruction) * line_count);

    // Conta número de processos individuais
    proc_count = count_processes_from_file(p_file, line_count);

    processes = malloc(sizeof(struct Process) * proc_count);

    // Cria árvore ou lista, dependendo da estratégia escolhida
    if (strategy == BUDDY)
    {
        memory_tree = memtree_create(memory_size);
    }
    else
    {
        memory_list = memlist_create(memory_size);
    }

    // Lê arquivo para adicionar processos e instrucoes em suas respectivas arrays
    read_file(p_file);

    fclose(p_file);    

    // Mostra a lista de processos e instruções
    if (dbg > 2)
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

    // Imprimem fragmentos de memória livre, antes das instruções
    if (strategy == BUDDY)
    {
        memtree_print(memory_tree);
    }
    else
    {
        memlist_print(memory_list);
    }

    // Debug 1 Imprime memória em blocos ou em hex de 1 byte (8 bits)
    if (dbg)
    {
        print_memory_blocks(allocated_blocks, memory_size);

        // Imprime memória alocada em bytes
        if (dbg == 2) { print_memory_bytes(memory_blocks, memory_size); }
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
                proc_start_address = (size_t)(memlist_add_worst(memory_list, instructions[i].pid, proc_size));
                break;
            case BUDDY:
                proc_start_address = (size_t)(memtree_add_buddy(memory_tree, instructions[i].pid, proc_size));
                break;
            }
            
            if (proc_start_address == -1)
            {
                printf("PROCESSO %s: TAMANHO %lu, NÃO ALOCADO. ESPAÇO INSUFICIENTE DE MEMÓRIA.\n", proc_name, proc_size);
            }
            else
            {
                size_t end_process = proc_start_address + proc_size;
                for (size_t j = proc_start_address; j < end_process; j++)
                {
                    memory_blocks[j] = rand() % 256;
                    allocated_blocks[j] = ALLOC;
                }

                size_t end_partition = end_process - 1;

                // Aloca resto da partição como inutilizável (fragmentação interna)
                if (strategy == BUDDY)
                {
                    // Encontra tamanho da partição alocada com operações bitwise
                    size_t temp = proc_size;
                    size_t partition_size = 1;
                    temp = (temp <<= 1) - 1;
                    while (temp >>= 1) { partition_size <<= 1; }

                    // Encontra endereço final da partição
                    end_partition = proc_start_address + partition_size - 1;

                    for (size_t j = end_process; j <= end_partition; j++)
                    {
                        allocated_blocks[j] = UNUSED;
                    }
                }

                printf("PROCESSO %s: TAMANHO %lu, INSERIDO NO ENDEREÇO 0x%07zX (%lu) - 0x%07zX (%lu).\n",
                        proc_name, proc_size, proc_start_address, proc_start_address, end_partition, end_partition);
            }
        }
        else    // OUT(proc)
        {
            switch (strategy)
            {
            case BUDDY:
                proc_start_address = (size_t)(memtree_remove_node(memory_tree, instructions[i].pid, proc_size));
                break;
            default:
                proc_start_address = (size_t)(memlist_remove_node(memory_list, instructions[i].pid));
                break;
            }

            if (proc_start_address == -1)
            {
                printf("PROCESSO %s: TAMANHO %lu, NÃO ENCONTRADO PARA REMOÇÃO.\n", proc_name, proc_size);
            }
            else
            {
                size_t end_process = proc_start_address + proc_size;
                for (int j = proc_start_address; j < end_process; j++)
                {
                    allocated_blocks[j] = DISALLOC;
                }

                size_t end_partition = end_process - 1;

                // Desaloca resto da partição inutilizada da partição pelo processo
                if (strategy == BUDDY)
                {

                    // Encontra tamanho da partição alocada com operações bitwise
                    size_t temp = proc_size;
                    size_t partition_size = 1;
                    temp = (temp <<= 1) - 1;
                    while (temp >>= 1) { partition_size <<= 1; }

                    // Encontra endereço final da partição
                    end_partition = proc_start_address + partition_size - 1;

                    for (size_t j = end_process; j <= end_partition; j++)
                    {
                        allocated_blocks[j] = DISALLOC;
                    }
                }

                printf("PROCESSO %s: TAMANHO %lu, REMOVIDO DO ENDEREÇO 0x%07zX (%lu) - 0x%07zX (%lu).\n",
                       proc_name, proc_size, proc_start_address, proc_start_address, end_partition, end_partition);
            }
        }

        // Imprime fragmentos de memória livre
        if (strategy == BUDDY)
        {
            memtree_print(memory_tree);
        }
        else
        {
            memlist_print(memory_list);
        }

        // Debug 1 Imprime memória em blocos ou em hex de 1 byte (8 bits)
        if (dbg)
        {
            print_memory_blocks(allocated_blocks, memory_size);

            if (dbg == 2) { print_memory_bytes(memory_blocks, memory_size); }
        }

        // Debug 2 Imprime nodos
        if (dbg == 3)
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

    // "Desaloca" toda memória mudando todos booleans para desalocados
    // Memória física continua com o que tinha nos blocos, que agora é garbage
    for (size_t i = 0; i < memory_size; i++)
    {
        allocated_blocks[i] = DISALLOC;
    }

    // Libera toda memória real do programa
    if (strategy == BUDDY)
    {
        memtree_clear(memory_tree);
        free(memory_tree);
    }
    else
    {
        memlist_clear(memory_list);
        free(memory_list);
    }
    free(allocated_blocks);
    free(memory_blocks);
    free(instructions);
    free(processes);

    #ifdef _WIN32
    SetConsoleOutputCP(old_page);
    #endif

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
                printf("Instrução inválida.\nEncerrando programa...\n");
                return 0;
            }

            instructions[num_instructions].operation = DISALLOC;
            instructions[num_instructions].pid = pid;
            num_instructions++;

            continue;
        }

        printf("Instrução inválida.\nEncerrando programa...\n");
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
    char **process_names = malloc(sizeof(char *) * line_count);

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
        process_names[i] = NULL;
    }
    free(process_names);
    process_names = NULL;

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
void print_memory_blocks(BYTE *allocated_blocks, size_t memory_size)
{
    for(size_t i = 0; i < memory_size; i++)
    {
        if (i%64 == 0) // Cada 64 bytes (cada byte sendo 00-FF, ou seja, 0-256)
        {
            printf("0x%07zX ", i * 256);  // Cada fila de Hex vai de 0x0000 até 0x4000 (0-16383, 64 bytes)
        }

        if (allocated_blocks[i] == ALLOC)
        {
            printf("█");
        }
        else if (allocated_blocks[i] == UNUSED)
        {
            printf("▓");
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