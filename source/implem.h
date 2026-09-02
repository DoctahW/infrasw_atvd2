#ifndef IMPLEM_H
#define IMPLEM_H
#include <pthread.h>

typedef struct {
    unsigned char *pixels;
    int largura, altura, max_iteracoes;
    int y_ini, y_fim;
} ThreadData;

typedef struct {
    unsigned char *pixels;
    int largura, altura, max_iteracoes;
    int *proxima;
    pthread_mutex_t *mutex;
} FilaData;

int mandel_serial(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

int mandel_openmp(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

int mandel_pthreads1(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

int mandel_pthreads2(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

#endif
