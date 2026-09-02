#include "mandel.h"
#include "implem.h"
#include <stdlib.h>
#include <omp.h>
#include <stdio.h>

static void *construct_structs_pth1(void *arg) {
    ThreadData *f = arg;
    for (int y = f->y_ini; y < f->y_fim; y++)
        for (int x = 0; x < f->largura; x++)
            mandel_pixel(f->pixels, x, y, f->largura, f->altura, f->max_iteracoes);
    return NULL;
}

static void *fila_pt(void *arg) {
    FilaData *d = arg;
    for (;;) {
        pthread_mutex_lock(d->mutex);
        int y = (*d->proxima)++;
        pthread_mutex_unlock(d->mutex);

        if (y >= d->altura)
            break;

        for (int x = 0; x < d->largura; x++)
            mandel_pixel(d->pixels, x, y, d->largura, d->altura, d->max_iteracoes);
    }
    return NULL;
}

int mandel_serial(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads) {
    (void)n_threads;
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            mandel_pixel(pixels, x, y, largura, altura, max_iteracoes);
        }
    }
    return 0;
}

int mandel_openmp(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads) {
    #pragma omp parallel for num_threads(n_threads) schedule(static)
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            mandel_pixel(pixels, x, y, largura, altura, max_iteracoes);
        }
    }
    return 0;
}

int mandel_pthreads1(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads) {
    if (n_threads > altura)
        n_threads = altura;

    pthread_t *tid = malloc((size_t)n_threads * sizeof *tid);
    ThreadData *faixa = malloc((size_t)n_threads * sizeof *faixa);
    int *criada = calloc((size_t)n_threads, sizeof *criada);
    if (tid == NULL || faixa == NULL || criada == NULL) {
        fprintf(stderr, "Falha ao alocar memória para threads.\n");
        free(tid);
        free(faixa);
        free(criada);
        return -1;
    }

    int base = altura / n_threads;
    int resto = altura % n_threads;

    int y = 0;
    int erro = 0;
    for (int i = 0; i < n_threads; i++) {
        int linhas = base + (i < resto ? 1 : 0);
        faixa[i] = (ThreadData){pixels, largura, altura, max_iteracoes, y, y + linhas};
        y += linhas;

        criada[i] = (pthread_create(&tid[i], NULL, construct_structs_pth1, &faixa[i]) == 0);
        if (!criada[i]) {
            fprintf(stderr, "Falha ao criar thread %d.\n", i);
            erro = 1;
            break;
        }
    }

    for (int i = 0; i < n_threads; i++) {
        if (criada[i])
            pthread_join(tid[i], NULL);
    }

    free(tid);
    free(faixa);
    free(criada);
    return erro ? -1 : 0;
}

int mandel_pthreads2(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads) {
    if (n_threads > altura)
        n_threads = altura;

    pthread_t *tid   = malloc((size_t)n_threads * sizeof *tid);
    int       *criada = calloc((size_t)n_threads, sizeof *criada);
    if (tid == NULL || criada == NULL) {
        fprintf(stderr, "Falha ao alocar memória para threads.\n");
        free(tid);
        free(criada);
        return -1;
    }

    int proxima = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    FilaData dados = { pixels, largura, altura, max_iteracoes, &proxima, &mutex };

    int erro = 0;
    for (int i = 0; i < n_threads; i++) {
        criada[i] = (pthread_create(&tid[i], NULL, fila_pt, &dados) == 0);
        if (!criada[i]) {
            fprintf(stderr, "Falha ao criar thread %d.\n", i);
            erro = 1;
            break;
        }
    }

    for (int i = 0; i < n_threads; i++)
        if (criada[i])
            pthread_join(tid[i], NULL);

    pthread_mutex_destroy(&mutex);
    free(tid);
    free(criada);
    return erro ? -1 : 0;
}
