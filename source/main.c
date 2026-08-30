#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mandel.h"
#include "implem.h"

typedef struct { 
    const char *rotulo;
    double s;
} Medida;

double agora_segundos(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

int checa_converte(char argv[]){
    char *fim;
    errno = 0;
    long valor = strtol(argv, &fim, 10);

    if (fim == argv){
        fprintf(stderr, "Erro: '%s' nao e um numero valido.\n", argv);
        return -1;
    }
    
    if (*fim != '\0') {
        fprintf(stderr, "Erro: '%s' nao e um numero valido.\n", argv);
        return -1;
    }

    if (errno == ERANGE){
        fprintf(stderr, "Erro: '%s' é um numero muito grande. (OVERFLOW)\n", argv);
        return -1;
    }

    if (valor <= 0 || valor > INT_MAX){
        fprintf(stderr, "Erro: '%s' não é um numero válido.\n", argv);
        return -1;
    }

    return (int)valor;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <largura> <altura> <iteracoes> <n_threads>\n", argv[0]);
        return 1;
    }

    int args[4];
    for (int i = 0; i < 4; i++) {
        args[i] = checa_converte(argv[i + 1]);
        if (args[i] < 0)
            return 1;
    }
    
    int largura = args[0];
    int altura = args[1];
    int iteracoes = args[2];
    int n_threads = args[3];

    Medida medidas[4];
    int n_medidas = 0;

    unsigned char *pixels = mandel_aloca(largura, altura);
    if (pixels == NULL) {
        fprintf(stderr, "Erro: nao foi possivel alocar memoria para a imagem.\n");
        return 1;
    }

    double inicio = agora_segundos();
    mandel_serial(pixels, largura, altura, iteracoes, n_threads);
    double tempo_serial = agora_segundos() - inicio;
    medidas[n_medidas++] = (Medida){"Serial", tempo_serial};

    if (mandel_escreve("mandelbrot_jems2_serial.pgm", pixels, largura, altura) !=0){
        fprintf(stderr, "Erro: nao foi possivel escrever a imagem.\n");
        free(pixels);
        return 1;
    }

    free(pixels);

    FILE *f = fopen("times.txt", "w");
    if (!f) { perror("times.txt"); return 1; }
    for (int i = 0; i < n_medidas; i++)
        fprintf(f, "%s: %.6fs\n", medidas[i].rotulo, medidas[i].s);
    fclose(f);
}