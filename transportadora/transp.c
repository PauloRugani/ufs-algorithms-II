#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct {
    char id[32];
    float valor, peso, volume;
    int alocado;
} Item;

typedef struct {
    char placa[32];
    float peso_max, volume_max;
    float peso_orig, volume_orig; // para porcentagem
    Item **itens;
    int n_itens;
} Veiculo;

void carregar_dados(FILE *f, Veiculo **v, int *nV, Item **it, int *nI) {
    if (fscanf(f, "%d", nV) != 1) { *nV = 0; return; }
    *v = malloc(sizeof(Veiculo) * (*nV));
    for (int i = 0; i < *nV; i++) {
        fscanf(f, "%31s %f %f", (*v)[i].placa, &(*v)[i].peso_max, &(*v)[i].volume_max);
        (*v)[i].peso_orig = (*v)[i].peso_max;
        (*v)[i].volume_orig = (*v)[i].volume_max;
        (*v)[i].itens = NULL; // alocar depois com tamanho exato
        (*v)[i].n_itens = 0;
    }

    if (fscanf(f, "%d", nI) != 1) { *nI = 0; return; }
    *it = malloc(sizeof(Item) * (*nI));
    for (int i = 0; i < *nI; i++) {
        fscanf(f, "%31s %f %f %f", (*it)[i].id, &(*it)[i].valor, &(*it)[i].peso, &(*it)[i].volume);
        (*it)[i].alocado = 0;
    }

    // agora que sabemos nI, alocar vetor de ponteiros para cada veículo com tamanho nI
    for (int i = 0; i < *nV; i++) {
        (*v)[i].itens = malloc(sizeof(Item*) * (*nI));
        (*v)[i].n_itens = 0;
    }
}

// Versão Knapsack 0/1 para cada veículo (DP contíguo)
void alocar_veiculo(Veiculo *v, Item *itens, int nItens) {
    int W = (int)floorf(v->peso_max);
    int V = (int)floorf(v->volume_max);
    if (W < 0) W = 0;
    if (V < 0) V = 0;

    // alocar bloco contíguo: (nItens+1) * (W+1) * (V+1)
    size_t dim_i = (size_t)(nItens + 1);
    size_t dim_w = (size_t)(W + 1);
    size_t dim_v = (size_t)(V + 1);
    size_t total = dim_i * dim_w * dim_v;

    // proteger contra alocações gigantescas
    float *block = calloc(total, sizeof(float));
    if (!block) {
        // fallback: sem alocação, não aloca itens
        return;
    }

    // função inline de indexação
    #define IDX(i,w,vol) ( (size_t)(i) * dim_w * dim_v + (size_t)(w) * dim_v + (size_t)(vol) )

    // DP
    for (int i = 1; i <= nItens; i++) {
        int pi = (int)floorf(itens[i-1].peso);
        int vi = (int)floorf(itens[i-1].volume);
        float val = itens[i-1].valor;

        if (itens[i-1].alocado) {
            // copiar camada anterior para a atual (memcpy mais rápido)
            float *src = block + IDX(i-1, 0, 0);
            float *dst = block + IDX(i, 0, 0);
            memcpy(dst, src, dim_w * dim_v * sizeof(float));
            continue;
        }

        // para cada w,vol calcular
        for (int w = 0; w <= W; w++) {
            for (int v_vol = 0; v_vol <= V; v_vol++) {
                float prev = block[IDX(i-1, w, v_vol)];
                if (w >= pi && v_vol >= vi) {
                    float cand = val + block[IDX(i-1, w - pi, v_vol - vi)];
                    block[IDX(i, w, v_vol)] = cand > prev ? cand : prev;
                } else {
                    block[IDX(i, w, v_vol)] = prev;
                }
            }
        }
    }

    // Reconstrução dos itens escolhidos (mesma ordem de seleção)
    int w = W, v_vol = V;
    for (int i = nItens; i > 0 && w > 0 && v_vol > 0; i--) {
        if (itens[i-1].alocado) continue;
        float cur = block[IDX(i, w, v_vol)];
        float prev = block[IDX(i-1, w, v_vol)];
        if (cur != prev) {
            v->itens[v->n_itens++] = &itens[i-1];
            itens[i-1].alocado = 1;
            w -= (int)floorf(itens[i-1].peso);
            v_vol -= (int)floorf(itens[i-1].volume);
            if (w < 0) w = 0;
            if (v_vol < 0) v_vol = 0;
        }
    }

    free(block);
    #undef IDX
}

void alocar_itens(Veiculo *v, int nV, Item *itens, int nI) {
    for (int i = 0; i < nV; i++)
        alocar_veiculo(&v[i], itens, nI);
}

void imprimir_veiculos(FILE *out, Veiculo *v, int nV) {
    for (int i = 0; i < nV; i++) {
        float val = 0, p = 0, vol = 0;
        for (int j = 0; j < v[i].n_itens; j++) {
            val += v[i].itens[j]->valor;
            p += v[i].itens[j]->peso;
            vol += v[i].itens[j]->volume;
        }
        fprintf(out, "[%s]R$%.2f,%.0fKG(%d%%),%.0fL(%d%%)->",
                v[i].placa,
                val,
                p,
                (int)round(p / v[i].peso_orig * 100),
                vol,
                (int)round(vol / v[i].volume_orig * 100));
        for (int j = v[i].n_itens - 1; j >= 0; j--) {
            if (j < v[i].n_itens - 1) fprintf(out, ",");
            fprintf(out, "%s", v[i].itens[j]->id);
        }
        fprintf(out, "\n");
    }
}

void imprimir_pendentes(FILE *out, Item *it, int nI) {
    float val = 0, p = 0, vol = 0;
    int first = 1;
    for (int i = 0; i < nI; i++) {
        if (!it[i].alocado) {
            val += it[i].valor;
            p += it[i].peso;
            vol += it[i].volume;
        }
    }
    fprintf(out, "PENDENTE:R$%.2f,%.0fKG,%.0fL->", val, p, vol);
    for (int i = 0; i < nI; i++) {
        if (!it[i].alocado) {
            if (!first) fprintf(out, ",");
            fprintf(out, "%s", it[i].id);
            first = 0;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    FILE *f_in = fopen(argv[1], "r");
    FILE *f_out = fopen(argv[2], "w");
    if (!f_in || !f_out) return 1;

    Veiculo *v; int nV;
    Item *it; int nI;
    carregar_dados(f_in, &v, &nV, &it, &nI);
    alocar_itens(v, nV, it, nI);
    imprimir_veiculos(f_out, v, nV);
    imprimir_pendentes(f_out, it, nI);
    return 0;
}