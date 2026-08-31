#include "mandel.h"
#include <omp.h>

void mandel_serial(unsigned char *pixels, int largura, int altura, 
    int max_iteracoes, int n_threads) {
    (void)n_threads;
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            mandel_pixel(pixels, x, y, largura, altura, max_iteracoes);
        }
    }
}

void mandel_openmp(unsigned char *pixels, int largura, int altura, 
    int max_iteracoes, int n_threads) {
    #pragma omp parallel for num_threads(n_threads) schedule(static)
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            mandel_pixel(pixels, x, y, largura, altura, max_iteracoes);
        }
    }
}
