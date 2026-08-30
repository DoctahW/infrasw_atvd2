#include <complex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "mandel.h"


int mandel_itera(double complex c, int max_iteracoes) {
    double complex z = 0;

    for (int i = 0; i < max_iteracoes; i++) {
        z = (z * z) + c;

        double re = creal(z), im = cimag(z);
        if (re * re + im * im > 4.0)
            return i + 1;
    }

    return max_iteracoes;
}

double complex mandel_mapeia(int x, int y, int largura, int altura) {
    double real = REAL_MIN + (x / (double)largura) * (REAL_MAX - REAL_MIN);
    double imag = IMAG_MIN + (y / (double)altura)  * (IMAG_MAX - IMAG_MIN);
 
    return CMPLX(real, imag);
}

unsigned char mandel_intensidade(int iter, int max_iteracoes){
    if (max_iteracoes <= 0)
        return 0;
    
    return (unsigned char)(((long)iter * 255) / max_iteracoes);
}

void mandel_pixel(unsigned char *pixels, int x, int y,
    int largura, int altura, int max_iteracoes) {
        
    double complex c = mandel_mapeia(x, y, largura, altura);
    int iter = mandel_itera(c, max_iteracoes);
 
    pixels[(size_t)y * (size_t)largura + (size_t)x] =
        mandel_intensidade(iter, max_iteracoes);
}

unsigned char *mandel_aloca(int largura, int altura) {
    if (largura <= 0 || altura <= 0)
        return NULL;

    return malloc((size_t)largura * (size_t)altura);
}

int mandel_escreve(const char *caminho, const unsigned char *pixels, 
    int largura, int altura) {
    FILE *arquivo = fopen(caminho, "w");
    if (arquivo == NULL){
        return -1;
    }

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            if (x > 0)
                fputc(' ', arquivo);
            fprintf(arquivo, "%u", (unsigned)pixels[(size_t)y * 
                (size_t)largura + (size_t)x]);
        }
        if (y < altura - 1)
            fputc('\n', arquivo);
    }
    
    if (ferror(arquivo)) {
        fclose(arquivo);
        return -1;
    }
    
    return (fclose(arquivo) == 0) ? 0 : -1;
}


