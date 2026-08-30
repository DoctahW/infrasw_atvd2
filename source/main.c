#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "mandel.h"
#include "implem.h"

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

    unsigned char *pixels = mandel_aloca(largura, altura);
    if (pixels == NULL) {
        fprintf(stderr, "Erro: nao foi possivel alocar memoria para a imagem.\n");
        return 1;
    }
    
    mandel_serial(pixels, largura, altura, iteracoes, n_threads);

    if (mandel_escreve("mandelbrot_jems2_serial.pgm", pixels, largura, altura) !=0){
        fprintf(stderr, "Erro: nao foi possivel escrever a imagem.\n");
        free(pixels);
        return 1;
    }

    free(pixels);
    
}