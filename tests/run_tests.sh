#!/usr/bin/env bash
#
# run_tests.sh — verificação ponta a ponta da Implementação 2 (Mandelbrot).
#
# NOTA DE AUTORIA: script de teste escrito com auxílio de IA, uso autorizado pelo
# professor em 29/08 para os testes automatizados (F7). As implementações em
# source/ são de autoria própria. Ver §8 do relatório.
#
# Uso:  ./tests/run_tests.sh [--rapido]
#         --rapido  pula o caso 1000x1000x500 (o mais demorado)
#
# Sai com 0 se tudo passar, 1 se qualquer verificação falhar.

set -uo pipefail

cd "$(dirname "$0")/.." || exit 1

LOGIN=jems2
RAPIDO=0
[ "${1:-}" = "--rapido" ] && RAPIDO=1

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

falhas=0
total=0

ok()    { total=$((total+1)); printf '  \033[32mok\033[0m    %s\n' "$1"; }
falha() { total=$((total+1)); falhas=$((falhas+1)); printf '  \033[31mFALHA\033[0m %s\n' "$1"; }
secao() { printf '\n\033[1m%s\033[0m\n' "$1"; }

pgms() { ls mandelbrot_${LOGIN}_*.pgm 2>/dev/null; }

# ── 0. binário ───────────────────────────────────────────────────────────────
secao "0. Binário"
if [ -x ./mandelbrot ]; then
    ok "./mandelbrot existe e é executável"
else
    falha "./mandelbrot não encontrado — rode 'make' antes"
    exit 1
fi

# ── 1. vetores do professor ──────────────────────────────────────────────────
# Cada vetor traz o comando e o conteúdo esperado. Os rótulos NÃO são uniformes:
# teste1/2 usam "Comando de entrada:", teste3 usa "Comando:" — por isso o regex
# aceita as duas formas.
secao "1. Vetores do professor (tests/vetores/)"
for vetor in tests/vetores/teste*.txt; do
    [ -e "$vetor" ] || continue
    nome=$(basename "$vetor")

    cmd=$(grep -A1 -E '^Comando( de entrada)?:$' "$vetor" | sed -n '2p')
    args=${cmd#./mandelbrot }
    sed -n '/^Conteudo esperado:$/,$p' "$vetor" | tail -n +2 > "$TMP/esperado"

    if [ -z "$args" ] || [ ! -s "$TMP/esperado" ]; then
        falha "$nome: não consegui extrair comando ou conteúdo esperado"
        continue
    fi

    rm -f mandelbrot_${LOGIN}_*.pgm
    # shellcheck disable=SC2086
    if ! ./mandelbrot $args >"$TMP/out" 2>"$TMP/err"; then
        falha "$nome: './mandelbrot $args' saiu com erro"
        continue
    fi
    [ -s "$TMP/out" ] && falha "$nome: escreveu em stdout (R.10)"

    produzidos=$(pgms)
    if [ -z "$produzidos" ]; then
        falha "$nome: nenhum .pgm foi gerado"
        continue
    fi
    for p in $produzidos; do
        if cmp -s "$p" "$TMP/esperado"; then
            ok "$nome ($args): $p"
        else
            falha "$nome ($args): $p difere do esperado"
        fi
    done
done

# ── 2. consistência entre implementações ─────────────────────────────────────
# Tamanhos sem vetor de referência: o oráculo é o próprio serial. Cobre o aceite
# da F3 (1,2,4,8 threads) e os casos de borda da F4 (R.6 e threads > altura).
secao "2. Consistência entre implementações (oráculo = serial)"
casos=("10 7 40 4" "4 4 50 16" "4 4 50 1" "1000 1000 500 1" "1000 1000 500 2" \
       "1000 1000 500 4" "1000 1000 500 8")
for caso in "${casos[@]}"; do
    case "$caso" in 1000*) [ "$RAPIDO" = 1 ] && continue ;; esac

    rm -f mandelbrot_${LOGIN}_*.pgm
    # shellcheck disable=SC2086
    if ! ./mandelbrot $caso >"$TMP/out" 2>&1; then
        falha "'$caso' saiu com erro"
        continue
    fi
    [ -s "$TMP/out" ] && falha "'$caso': escreveu em stdout (R.10)"

    ref=mandelbrot_${LOGIN}_serial.pgm
    if [ ! -f "$ref" ]; then
        falha "'$caso': $ref não foi gerado"
        continue
    fi
    n=0
    for p in $(pgms); do
        [ "$p" = "$ref" ] && continue
        n=$((n+1))
        cmp -s "$p" "$ref" || { falha "'$caso': $p difere do serial"; continue 2; }
    done
    if [ "$n" -eq 0 ]; then
        ok "'$caso': serial gerado (sem outra implementação para comparar ainda)"
    else
        ok "'$caso': as $((n+1)) imagens são idênticas"
    fi
done

# ── 3. tabela de erros (§10 do PLANO) ────────────────────────────────────────
secao "3. Tabela de erros (§10)"
espera_erro() {
    local desc=$1; shift
    ./mandelbrot "$@" >"$TMP/out" 2>"$TMP/err"
    local code=$?
    if [ "$code" -eq 0 ]; then
        falha "$desc: deveria falhar, saiu com 0"
    elif [ ! -s "$TMP/err" ]; then
        falha "$desc: exit=$code mas nada em stderr"
    elif [ -s "$TMP/out" ]; then
        falha "$desc: escreveu em stdout"
    else
        ok "$desc (exit=$code, erro em stderr)"
    fi
}
espera_erro "#1  sem argumentos"
espera_erro "#2  poucos argumentos"   4 4 50
espera_erro "#3  argumentos demais"   4 4 50 1 extra
espera_erro "#4  largura 0"           0 4 50 1
espera_erro "#5  altura -1"           4 -1 50 1
espera_erro "#6  iteracoes 0"         4 4 0 1
espera_erro "#7  threads 0"           4 4 50 0
espera_erro "#8  nao numerico"        abc 4 50 1
espera_erro "#9  fora de faixa"       99999999999 4 50 1
espera_erro "#9b overflow de long"    99999999999999999999 4 50 1
espera_erro "#8b numero fracionario"  4.5 4 50 1

# ── 4. falha de recurso: diretório sem permissão de escrita (§10 #10) ─────────
secao "4. Falha de recurso (§10 #10)"
ro="$TMP/somente-leitura"
mkdir -p "$ro" && cp ./mandelbrot "$ro/" && chmod 555 "$ro"
saida=$( cd "$ro" && ./mandelbrot 4 4 50 1 2>&1 >/dev/null; echo "exit=$?" )
code=${saida##*exit=}
if [ "$code" -ne 0 ]; then
    ok "#10 diretório sem permissão de escrita (exit=$code, erro em stderr)"
else
    falha "#10 diretório sem permissão de escrita: saiu com 0"
fi
chmod 755 "$ro"

# ── 5. formato do times.txt ──────────────────────────────────────────────────
secao "5. Formato do times.txt"
./mandelbrot 4 4 50 1 >/dev/null 2>&1
if [ ! -f times.txt ]; then
    falha "times.txt não foi gerado"
elif grep -qvE '^(Serial|OpenMP|Pthreads1|Pthreads2): [0-9]+\.[0-9]{6}s$' times.txt; then
    falha "times.txt fora do formato esperado:"; sed 's/^/        /' times.txt
else
    ok "times.txt no formato ($(wc -l < times.txt) linha(s))"
fi
./mandelbrot 4 4 50 1 >/dev/null 2>&1
if [ "$(sort -u times.txt | wc -l)" -eq "$(wc -l < times.txt)" ]; then
    ok "duas execuções seguidas não acumulam linhas"
else
    falha "times.txt acumulou linhas entre execuções (abrir em \"w\", não \"a\")"
fi

# ── resumo ───────────────────────────────────────────────────────────────────
printf '\n────────────────────────────────────────\n'
if [ "$falhas" -eq 0 ]; then
    printf '\033[32mTODOS OS %d TESTES PASSARAM\033[0m\n' "$total"
    exit 0
else
    printf '\033[31m%d de %d FALHARAM\033[0m\n' "$falhas" "$total"
    exit 1
fi
