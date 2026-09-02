#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mandel.h"
#include "implem.h"

typedef int (*Mandel_F)(unsigned char *, int, int, int, int);

typedef struct {
    const char *rotulo;
    const char *slug;
    Mandel_F    funcao;
} Implem;

typedef struct {
    const char *rotulo;
    double s;
} Medida;

double agora_segundos(void) {
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

Medida executa_mandelbrot(const Implem *impl, int largura, int altura, int iteracoes, int n_threads){
    unsigned char *pixels = mandel_aloca(largura, altura);
    if (pixels == NULL) {
        fprintf(stderr, "Erro: nao foi possivel alocar memoria.\n");
        return (Medida){impl->rotulo, -1};
    }

    double inicio = agora_segundos();
    if (impl->funcao(pixels, largura, altura, iteracoes, n_threads) != 0) {
        fprintf(stderr, "Erro: %s falhou na execucao.\n", impl->rotulo);
        free(pixels);
        return (Medida){impl->rotulo, -1};
    }
    double tempo = agora_segundos() - inicio;

    char arquivo[64];
    snprintf(arquivo, sizeof(arquivo), "mandelbrot_jems2_%s.pgm", impl->slug);

    if (mandel_escreve(arquivo, pixels, largura, altura) != 0) {
        fprintf(stderr, "Erro: nao foi possivel escrever '%s'.\n", arquivo);
        free(pixels);
        return (Medida){impl->rotulo, -1};
    }

    free(pixels);
    return (Medida){impl->rotulo, tempo};
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

    static const Implem implementacoes[] = {
        { "Serial",  "serial",  mandel_serial  },
        { "OpenMP",  "openmp",  mandel_openmp  },
        { "Pthreads1", "pthreads1", mandel_pthreads1 },
        { "Pthreads2", "pthreads2", mandel_pthreads2 },
    };
    enum { N_IMPLEMS = (int)(sizeof implementacoes / sizeof implementacoes[0]) };

    Medida medidas[N_IMPLEMS];
    int n_medidas = 0;

    for (int i = 0; i < N_IMPLEMS; i++) {
        Medida m = executa_mandelbrot(&implementacoes[i], largura, altura,
            iteracoes, n_threads);

        if (m.s < 0)
            return 1;
        medidas[n_medidas++] = m;
    }

    FILE *f = fopen("times.txt", "w");
    if (!f) { perror("times.txt"); return 1; }
    for (int i = 0; i < n_medidas; i++)
        fprintf(f, "%s: %.6fs\n", medidas[i].rotulo, medidas[i].s);
    fclose(f);
    return 0;
}
