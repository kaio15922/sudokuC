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

int main()
{
    clock_t tempo_total_inicio = clock();

    clock_t lista_inicio, lista_fim;
    clock_t ordenacao_inicio, ordenacao_fim;
    clock_t busca_inicio, busca_fim;

    int capacidade = 9000000;

    struct sudoku *listaBinaria =
        malloc(capacidade * sizeof(struct sudoku));

    if(listaBinaria == NULL)
    {
        printf("Erro de memoria\n");
        return 1;
    }

    FILE *arquivo = fopen("sudoku.csv", "r");

    if(arquivo == NULL)
    {
        printf("Erro ao abrir arquivo\n");
        return 1;
    }

    char linha[200];

    int quantidade = 0;

    lista_inicio = clock();
    quantidade = carregarDados(listaBinaria, arquivo);
    lista_fim = clock();

    fclose(arquivo);

    printf("Lidos: %d\n", quantidade);

    ordenacao_inicio = clock();
    quicksort(listaBinaria, 0, quantidade - 1);
    ordenacao_fim = clock();

    char input[100];

    printf("Digite o seu sudoku: ");
    scanf("%83s", input);

    busca_inicio = clock();
    int achou = BuscaBinaria(listaBinaria, quantidade - 1, input);
    busca_fim = clock();

    if(achou == -1)
    {
        printf("Não achou nada correspondente.\n");
    }
    else
    {
        printf("A solucao eh: %s\n", listaBinaria[achou].solution);
        
        // 4. Teste de Remoção Automática (Para provar o funcionamento ao professor)
        printf("\n[Testando Remocao] Removendo o sudoku que acoramos de encontrar...\n");
        if (removerSudoku(listaBinaria, &quantidade, input)) 
        {
            printf("Sucesso! O sudoku foi removido. Nova quantidade: %d\n", quantidade);
        }
    }

    free(listaBinaria);
    clock_t tempo_total_fim = clock();

    // --- RELATÓRIO DE RUNTIME ---
    printf("\n\n=========================================\n");
    printf("         RELATÓRIO DE RUNTIME\n");
    printf("=========================================\n");
    printf("Tempo para criar a lista:     %f segundos\n", (double)(lista_fim - lista_inicio) / CLOCKS_PER_SEC);
    printf("Tempo para ordenar:           %f segundos\n", (double)(ordenacao_fim - ordenacao_inicio) / CLOCKS_PER_SEC);
    printf("Tempo da ultima busca:        %f segundos\n", (double)(busca_fim - busca_inicio) / CLOCKS_PER_SEC);
    printf("-----------------------------------------\n");
    printf("TEMPO TOTAL DO PROGRAMA:      %f segundos\n", (double)(tempo_total_fim - tempo_total_inicio) / CLOCKS_PER_SEC);
    printf("=========================================\n");

    return 0;
}