#ifndef MANDEL_H
#define MANDEL_H
#include <complex.h>

#define REAL_MIN -2.0
#define REAL_MAX 1.0
#define IMAG_MIN -1.5
#define IMAG_MAX 1.5

double complex mandel_mapeia(int x, int y, int largura, int altura);

int mandel_itera(double complex c, int max_iteracoes);

unsigned char mandel_intensidade(int iter, int max_iteracoes);

void mandel_pixel(unsigned char *pixels, int x, int y,
    int largura, int altura, int max_iteracoes);

unsigned char *mandel_aloca(int largura, int altura);

int mandel_escreve(const char *caminho, const unsigned char *pixels, 
    int largura, int altura);

#endif