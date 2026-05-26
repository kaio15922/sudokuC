#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct sudoku
{
    char puzzle[100];
    char solution[100];
};

int comparar(char *string1, char *string2)
{
    for(int i = 0; i < 81; i++)
    {
        if((string1[i] - '0') > (string2[i] - '0'))
        {
            return 0;
        }

        if((string1[i] - '0') < (string2[i] - '0'))
        {
            return 1;
        }
    }

    return -1;
}

int BuscaBinaria(struct sudoku ar[], int t, char * ipt)
{
    int ini = 0, fim = t-1, meio;

    while(ini <= fim){
        meio = ini + (fim - ini)/2;

        if (comparar(ar[meio].puzzle, ipt) == -1)
        {
            return meio; //encontrou
        }

        if (!comparar(ar[meio].puzzle, ipt))
        {
            fim = meio - 1;
        }
        else
        {
            ini = meio + 1;
        }
    }

    return -1;
}

void trocar(struct sudoku *a, struct sudoku *b)
{
    struct sudoku temp = *a;
    *a = *b;
    *b = temp;
}

int particao(struct sudoku ar[], int baixo, int alto)
{
    char *pivo = ar[alto].puzzle;

    int i = baixo - 1;

    for(int j = baixo; j < alto; j++)
    {
        int comp = comparar(ar[j].puzzle, pivo);

        if(comp == 1 || comp == -1)
        {
            i++;
            trocar(&ar[i], &ar[j]);
        }
    }

    trocar(&ar[i + 1], &ar[alto]);

    return i + 1;
}

void quicksort(struct sudoku ar[], int baixo, int alto)
{
    if(baixo < alto)
    {
        int pi = particao(ar, baixo, alto);

        quicksort(ar, baixo, pi - 1);
        quicksort(ar, pi + 1, alto);
    }
}

int carregarDados(struct sudoku ar[], FILE *arquivo)
{
    char linha[200];
    int qtd = 0;
    
    while(fgets(linha, sizeof(linha), arquivo))
    {
        sscanf(linha, "%[^,],%s", ar[qtd].puzzle, ar[qtd].solution);
        qtd++;
    }
    
    return qtd; // a quantidade total de elementos lidos
}

int removerSudoku(struct sudoku ar[], int *quantidade, char *ipt)
{
    // busca Binária para achar a posição do puzzle
    int indice = BuscaBinaria(ar, *quantidade, ipt);

    // não existe na tabela, retorna 0 (falha)
    if (indice == -1)
    {
        return 0;
    }

    // shift
    for (int i = indice; i < (*quantidade) - 1; i++)
    {
        ar[i] = ar[i + 1];
    }

    (*quantidade)--;

    return 1;
}

int main(void)
{
    //Desativa o buffer do c (pode dar erros na hora de passar os dados pro arquivo da interface)
    setvbuf(stdout, NULL, _IONBF, 0);
   
    //Variaveis para determinar efiencia:
    clock_t lista_inicio, lista_fim;
    clock_t ordenacao_inicio, ordenacao_fim;
    clock_t busca_inicio, busca_fim;

    int capacidade = 9000000;

    //Tenta criar listaBinaria
    struct sudoku *listaBinaria =
        malloc(capacidade * sizeof(struct sudoku));

    if(listaBinaria == NULL)
    {
        printf("Erro de memoria\n");
        return 1;
    }

    //Tenta abrir o arquivo csv
    FILE *arquivo = fopen("sudoku.csv", "r");
    if(arquivo == NULL)
    {
        arquivo = fopen("../sudoku.csv", "r"); // Se não achar, tenta na pasta anterior
    }

    if(arquivo == NULL)
    {
        printf("ERRO_ARQUIVO\n");
        fflush(stdout);
        free(listaBinaria);
        return 1;
    }

    int quantidade = 0;

    //Medindo tempo de carregamento do .csv:
    lista_inicio = clock();
    quantidade = carregarDados(listaBinaria, arquivo);
    lista_fim = clock();
    fclose(arquivo);
    double tempo_carregamento = (double)(lista_fim - lista_inicio) / CLOCKS_PER_SEC;

    //Medindo tempo de ordenacao:
    ordenacao_inicio = clock();
    quicksort(listaBinaria, 0, quantidade - 1);
    ordenacao_fim = clock();

    double tempo_ordenacao = (double)(ordenacao_fim - ordenacao_inicio) / CLOCKS_PER_SEC;

    // Avisa o Python que a carga e a ordenação terminaram com sucesso!
    printf("PRONTO\n");
    fflush(stdout); // Força o Windows a enviar o "PRONTO" na mesma hora para o Python!

    char input[100];

    // Loop de escuta do Python
    while (scanf("%99s", input) != EOF) 
    {
        // Se o comando começar com 'R', o Python quer remover
        if (input[0] == 'R') 
        {
            char *puzzle_para_remover = &input[1]; // Pula o 'R' para pegar só os 81 números
            busca_inicio = clock();
            int removido = removerSudoku(listaBinaria, &quantidade, puzzle_para_remover);
            busca_fim = clock();

            if (removido == 1) {
                printf("REMOVIDO\n");
            } else {
                printf("ERRO_REMOVER\n");
            }
        } 
        // Caso contrário, é uma BUSCA normal
        else 
        {
            busca_inicio = clock();
            int achou = BuscaBinaria(listaBinaria, quantidade, input); 
            busca_fim = clock();

            if(achou == -1) {
                printf("ERRO\n");
            } else {
                printf("%s\n", listaBinaria[achou].solution);
            }
        }

        // Envia os tempos puros por linha para o arquivo .py 
        printf("%f\n", tempo_carregamento);
        printf("%f\n", tempo_ordenacao);
        printf("%f\n", (double)(busca_fim - busca_inicio) / CLOCKS_PER_SEC);
        fflush(stdout);
    }

    free(listaBinaria);
    return 0;
}