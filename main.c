#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>

//nivel maximo suficiente para nove milhoes de registros
#define MAX_NIVEL 23

//arquivo csv fica aberto globalmente para leitura em tempo de execucao
static FILE* g_csv   = NULL;
static int   g_total = 0;

//estrutura do no usando vetor flexivel para economizar memoria ram
struct No 
{
    int        id;
    int        nivel;
    long       offset;   
    struct No* prox[];   
};

//aloca memoria apenas para a quantidade exata de ponteiros que o no vai usar
struct No* criarNo(int id, int nivel, long offset) 
{
    struct No* no = calloc(1, sizeof(struct No) + (size_t)(nivel + 1) * sizeof(struct No*));
    if (!no) 
    { 
        fputs("Erro fatal: sem memoria.\n", stderr); exit(1); 
    }
    no -> id     = id;
    no -> nivel  = nivel;
    no -> offset = offset;
    return no;
}

//sorteia o nível usando moeda binaria limitado ao tamanho maximo configurado
int sortearNivel() 
{
    int n = 0;
    while ((rand() & 1) && n < MAX_NIVEL) 
    {
        n++;
    }
    return n;
}

//inicializa a cabeceira e a sentinela conectando todos os niveis da lista
void criarListaSalto(struct No** header, struct No** sentinela) 
{
    *header    = criarNo(-1,      MAX_NIVEL, -1);
    *sentinela = criarNo(INT_MAX, MAX_NIVEL, -1);
    for (int i = 0; i <= MAX_NIVEL; i++)
    {
        (*header) -> prox[i] = *sentinela;
    }
}

//insere o novo registro na lista saltando as camadas de forma ordenada por id
void adicionarNaLista(struct No* header, struct No* sentinela, int id, long offset) 
{
    int nivel = sortearNivel();

    struct No* update[MAX_NIVEL + 1];
    struct No* atual = header;

    for (int i = MAX_NIVEL; i >= 0; i--) 
    {
        while (atual -> prox[i] != sentinela && atual -> prox[i] -> id < id)
        {
            atual = atual -> prox[i];
        }
        update[i] = atual;
    }

    struct No* novo = criarNo(id, nivel, offset);

    for (int i = 0; i <= nivel; i++) 
    {
        novo -> prox[i]      = update[i] -> prox[i];
        update[i] -> prox[i] = novo;
    }
}

//busca pelo identificador numerico saltando as camadas superiores em tempo logaritmo
struct No* buscarPorID(struct No* header, struct No* sentinela, int id) 
{
    struct No* atual = header;

    for (int i = MAX_NIVEL; i >= 0; i--) 
    {
        while (atual -> prox[i] != sentinela && atual -> prox[i] -> id < id)
        {
            atual = atual -> prox[i];
        }
    }

    atual = atual -> prox[0];
    return (atual != sentinela && atual -> id == id) ? atual : NULL;
}

//faz a busca fisica no arquivo do disco pegando o texto na posicao do offset
int lerPuzzle(long offset, char* desafio, char* solucao) 
{
    if (!g_csv || offset < 0)                                      return 0;
    if (fseek(g_csv, offset, SEEK_SET) != 0)                       return 0;
    char linha[250];
    if (!fgets(linha, sizeof(linha), g_csv))                        return 0;
    if (sscanf(linha, "%81[^,],%81s", desafio, solucao) != 2) return 0;
    desafio[81] = solucao[81] = '\0';
    return 1;
}

//varre o arquivo csv mapeando apenas as posicoes de bytes para poupar memoria
void carregarDadosCSV(struct No* header, struct No* sentinela, const char* nomeArquivo) 
{
    g_csv = fopen(nomeArquivo, "r");
    if (!g_csv) 
    { 
        printf("Erro: nao foi possivel abrir %s\n", nomeArquivo); return; 
    }

    char linha[250];
    char v_desafio[100];
    char v_solucao[100];
    int  id = 1;

    printf("Iniciando leitura do arquivo... aguarde.\n");

    while (1) 
    {
        long pos = ftell(g_csv);              
        if (!fgets(linha, sizeof(linha), g_csv)) 
        {
            break;
        }

        if (sscanf(linha, "%81[^,],%99s", v_desafio, v_solucao) != 2) 
        {
            continue;
        }
        v_desafio[strcspn(v_desafio, "\r\n")] = '\0';
        if (strcmp(v_desafio, "puzzle") == 0) 
        {
            continue;   
        }

        adicionarNaLista(header, sentinela, id, pos);
        id++;

        if ((id - 1) % 1000000 == 0)
        {
            printf("  %d milhoes indexados...\n", (id - 1) / 1000000);
        }
    }

    g_total = id - 1;
    printf("Sucesso! %d sudokus indexados.\n", g_total);
}

//desenha as strings de oitenta e um caracteres no formato quadrado do jogo
void desenharTabuleiro(char* sudoku) 
{
    printf("-------------------------\n");
    for (int i = 0; i < 9; i++) 
    {
        printf("| ");
        for (int j = 0; j < 9; j++) 
        {
            char c = sudoku[i * 9 + j];
            printf("%c ", c == '0' ? '.' : c);
            if ((j + 1) % 3 == 0) 
            {
                printf("| ");
            }
        }
        printf("\n");
        if ((i + 1) % 3 == 0) 
        {
            printf("-------------------------\n");
        }
    }
}

//percorre a lista liberando a memoria de cada no alocado dinamicamente
void liberarLista(struct No* header, struct No* sentinela) 
{
    struct No* atual = header -> prox[0];
    while (atual != sentinela) 
    {
        struct No* prox = atual -> prox[0];
        free(atual);
        atual = prox;
    }
    free(header);
    free(sentinela);
}

int main() 
{
    srand((unsigned)time(NULL));

    //ponteiros principais para gerenciar as extremidades da lista de saltos
    struct No* header    = NULL;
    struct No* sentinela = NULL;
    //chama a funcao para criar as estruturas iniciais do ambiente
    criarListaSalto(&header, &sentinela);

    //variaveis de controle de tempo para monitorar o relogio do processador
    clock_t t0 = clock();
    //dispara a carga de dados mapeando o arquivo csv para dentro da lista
    carregarDadosCSV(header, sentinela, "sudoku.csv");
    //calcula a diferenca do tempo de carregamento e imprime na tela em segundos
    printf("Tempo de carga: %.2f s\n", (double)(clock() - t0) / CLOCKS_PER_SEC);

    //variavel de controle para gerenciar o laco do menu de perguntas
    int opcao = 1;
    //mantem o menu rodando enquanto o usuario escolher a opcao de continuar
    while (opcao == 1) 
    {
        printf("\n==================================================\n");
        printf("Digite o numero do puzzle (1 a %d):\n> ", g_total);
        int id;
        //captura o numero identificador da linha que o usuario deseja ver
        scanf("%d", &id);

        //valida se o numero digitado esta dentro dos limites reais do arquivo
        if (id < 1 || id > g_total) 
        {
            printf("Numero invalido. Escolha entre 1 e %d.\n", g_total);
        } 
        else 
        {
            char desafio[82], solucao[82];
            int achou_e_leu = 0;

            //marca o tempo de inicio unificado para medir a busca e a leitura de disco
            clock_t tb = clock();
            
            //primeiro faz a busca logaritmica rapida na memoria ram
            struct No* no = buscarPorID(header, sentinela, id);
            
            //se o no existir vai buscar o texto complementar direto no arquivo csv
            if (no) 
            {
                achou_e_leu = lerPuzzle(no -> offset, desafio, solucao);
            }
            
            //calcula a velocidade total combinando a busca mais a leitura fisica do arquivo
            double tempo = (double)(clock() - tb) / CLOCKS_PER_SEC;

            //se toda a operacao deu certo exibe os resultados formatados na tela
            if (achou_e_leu) 
            {
                printf("\n--- [PUZZLE #%d] ORIGINAL ---\n", id);
                //desenha o tabuleiro original com os espacos em branco formatados
                desenharTabuleiro(desafio);
                printf("\n--- SOLUCAO (recuperada e lida em %.6f s) ---\n", tempo);
                //desenha o tabuleiro resolvido que foi pescado do csv
                desenharTabuleiro(solucao);
            } 
            else 
            {
                printf("Puzzle %d nao encontrado ou erro ao ler o arquivo.\n", id);
            }
        }

        printf("\nBuscar outro? (1-Sim / 0-Sair): ");
        //recebe a resposta do usuario para continuar ou encerrar o programa
        scanf("%d", &opcao);
    }

    //se o ponteiro do arquivo global estiver aberto fecha o descritor
    if (g_csv) 
    {
        fclose(g_csv);
    }
    //devolve toda a memoria alocada dinamicamente de volta para o sistema
    liberarLista(header, sentinela);
    printf("Encerrado.\n");
    return 0;
}