#include <stdio.h>
#include <stdlib.h>
#include "memory.c"
#include "memory_linkedlist.h"

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

    FILE *file;

    if (argc == 3)
    {
        file = fopen(argv[1], "r");
    }
    else
    {
        printf("usage: main <parameter>\n");
        char input[100] = "";
        printf("Type the name of the file: ");
        scanf("%s", input);
        printf("\n");
        file = fopen(input, "r");
    }

    if (file == NULL)
    {
        printf("File not found.\nExiting program...\n");
        return 1;
    }

    struct Memory_block block_list;

    initialize_block_list(&block_list, 2048);
    
    // perguntar politica (circular ou worst fit)
    //int option;
    //printf("Qual política de alocação você gostaria de implementar? \n");
    //printf("(1) Circular Fit \n (2) Worst Fit")
    // scanf("%d", &option);
    

    //... lê arquivo
    

    return 0;
}


// worst fit

// circular fit