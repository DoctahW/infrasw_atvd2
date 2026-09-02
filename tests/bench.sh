#!/usr/bin/env bash
# Benchmark auxiliar: menor de 3 execucoes por numero de threads, em 1000x1000x500.
# Tempos vindos do times.txt do proprio binario (regiao so do calculo).
set -u
cd "$(dirname "$0")/.."
val() { sed -n "s/^$1: \([0-9.]*\)s$/\1/p" times.txt; }
min() { awk -v a="$1" -v b="$2" 'BEGIN{ if (b=="") print a; else print (a<b)?a:b }'; }

printf '%-5s %-9s %-9s %-9s %-9s\n' nthr Serial OpenMP Pthr1 Pthr2
for t in 1 2 4 8; do
    sm= ; om= ; p1m= ; p2m=
    for r in 1 2 3; do
        ./mandelbrot 1000 1000 500 "$t"
        sm=$(min "$(val Serial)" "$sm")
        om=$(min "$(val OpenMP)" "$om")
        p1m=$(min "$(val Pthreads1)" "$p1m")
        p2m=$(min "$(val Pthreads2)" "$p2m")
    done
    printf '%-5s %-9s %-9s %-9s %-9s\n' "$t" "$sm" "$om" "$p1m" "$p2m"
done
