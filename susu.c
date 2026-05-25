#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
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
        arquivo = fopen("../sudoku.csv", "r"); // Se não achar, tenta na pasta anterior
    }

    if(arquivo == NULL)
    {
        printf("Erro ao abrir arquivo\n");
        fflush(stdout);
        return 1;
    }

    char linha[200];

    int quantidade = 0;

    //Medindo tempo de carregamento do .csv:
    lista_inicio = clock();
    while(fgets(linha, sizeof(linha), arquivo))
    {
        sscanf(linha, "%[^,],%s", listaBinaria[quantidade].puzzle, listaBinaria[quantidade].solution);

        quantidade++;
    }
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
    fflush(stdout); // CRUCIAL: Força o Windows a enviar o "PRONTO" na mesma hora para o Python!

   char input[100];
    // Loop infinito: o programa em C fica vivo esperando perguntas do Python
    while (scanf("%83s", input) != EOF) 
    {
        busca_inicio = clock();
        int achou = BuscaBinaria(listaBinaria, quantidade, input); 
        busca_fim = clock();

        double tempo_busca = (double)(busca_fim - busca_inicio) / CLOCKS_PER_SEC;

        if (achou == -1) 
        {
            printf("ERRO:Nao encontrado\n");
        } 
        else 
        {
            printf("%s\n", listaBinaria[achou].solution);
        }
        
        // Envia as métricas estruturadas completando as 4 linhas esperadas pelo Python
        printf("%f\n", tempo_carregamento);
        printf("%f\n", tempo_ordenacao);
        printf("%f\n", tempo_busca);
        
        fflush(stdout); // Limpa o buffer a cada busca realizada
    }
    // Liberação de memória (só vai rodar se o programa for encerrado)
    free(listaBinaria);
    
    return 0;
}