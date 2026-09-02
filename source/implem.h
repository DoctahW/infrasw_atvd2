#ifndef IMPLEM_H
#define IMPLEM_H
int mandel_serial(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

int mandel_openmp(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

int mandel_pthreads1(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

#endif
