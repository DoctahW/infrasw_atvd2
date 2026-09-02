#ifndef IMPLEM_H
#define IMPLEM_H
void mandel_serial(unsigned char *pixels, int largura, int altura,
    int max_iteracoes, int n_threads);

void mandel_openmp(unsigned char *pixels, int largura, int altura, 
    int max_iteracoes, int n_threads);

void mandel_pthreads1(unsigned char *pixels, int largura, int altura, 
    int max_iteracoes, int n_threads);

#endif
