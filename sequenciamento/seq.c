#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    char* sequencia;
} Gene;

typedef struct {
    char* nome;
    Gene* genes;
    int num_genes;
    int prob;
    int order;
} Doenca;

void extrair_info(FILE *input, char **dna, int *tamSubcadeia, Doenca **doencas, int *numDoencas) {
    fscanf(input, "%d", tamSubcadeia);

    char buffer[1000005];
    fscanf(input, "%s", buffer);
    *dna = strdup(buffer);

    fscanf(input, "%d", numDoencas);
    *doencas = (Doenca*) malloc(sizeof(Doenca) * (*numDoencas));

    for (int i = 0; i < *numDoencas; i++) {
        int numGenes;
        fscanf(input, "%s %d", buffer, &numGenes);
        (*doencas)[i].nome = strdup(buffer);
        (*doencas)[i].num_genes = numGenes;
        (*doencas)[i].order = i;
        (*doencas)[i].prob = 0;
        (*doencas)[i].genes = (Gene*) malloc(sizeof(Gene) * numGenes);
        for (int j = 0; j < numGenes; j++) {
            fscanf(input, "%s", buffer);
            (*doencas)[i].genes[j].sequencia = strdup(buffer);
        }
    }
}

#define ALFABETO 4

int charToIndex(char c) {
    switch(c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        case 'T': return 3;
        default: return -1;
    }
}

typedef struct {
    int len;
    int link;
    int next[ALFABETO];
} SAState;

typedef struct {
    SAState* sa;
    int size;
    int last;
} SuffixAutomaton;

SuffixAutomaton* initSA(int maxSize) {
    SuffixAutomaton* sam = (SuffixAutomaton*) malloc(sizeof(SuffixAutomaton));
    sam->sa = (SAState*) malloc(sizeof(SAState) * maxSize);
    sam->size = 1;
    sam->last = 0;

    sam->sa[0].len = 0;
    sam->sa[0].link = -1;
    for (int i = 0; i < ALFABETO; i++)
        sam->sa[0].next[i] = -1;

    return sam;
}

void extendSA(SuffixAutomaton* sam, int c) {
    int cur = sam->size++;
    sam->sa[cur].len = sam->sa[sam->last].len + 1;
    for (int i = 0; i < ALFABETO; i++)
        sam->sa[cur].next[i] = -1;

    int p = sam->last;
    while (p != -1 && sam->sa[p].next[c] == -1) {
        sam->sa[p].next[c] = cur;
        p = sam->sa[p].link;
    }

    if (p == -1) {
        sam->sa[cur].link = 0;
    } else {
        int q = sam->sa[p].next[c];
        if (sam->sa[p].len + 1 == sam->sa[q].len) {
            sam->sa[cur].link = q;
        } else {
            int clone = sam->size++;
            sam->sa[clone] = sam->sa[q];
            sam->sa[clone].len = sam->sa[p].len + 1;
            sam->sa[q].link = sam->sa[cur].link = clone;
            while (p != -1 && sam->sa[p].next[c] == q) {
                sam->sa[p].next[c] = clone;
                p = sam->sa[p].link;
            }
        }
    }
    sam->last = cur;
}

SuffixAutomaton* buildSA(const char* dna) {
    int n = strlen(dna);
    SuffixAutomaton* sam = initSA(2*n + 5);
    for (int i = 0; i < n; i++)
        extendSA(sam, charToIndex(dna[i]));
    return sam;
}

int longestMatchFrom(const SuffixAutomaton* sam, const char* gene, int start) {
    int state = 0, length = 0;
    for (int j = start; gene[j]; j++) {
        int c = charToIndex(gene[j]);
        if (sam->sa[state].next[c] != -1) {
            state = sam->sa[state].next[c];
            length++;
        } else break;
    }
    return length;
}


int maxMatchDP(const SuffixAutomaton* sam, const char* gene, int tamSubcadeia) {
    int m = strlen(gene);
    int* dp = (int*) calloc(m + 1, sizeof(int));
    int* L = (int*) malloc(sizeof(int) * m);

    for (int i = 0; i < m; i++)
        L[i] = longestMatchFrom(sam, gene, i);

    for (int i = m - 1; i >= 0; i--) {
        int best = dp[i + 1];
        if (L[i] >= tamSubcadeia) {
            for (int seg = tamSubcadeia; seg <= L[i]; seg++)
                if (seg + dp[i + seg] > best) best = seg + dp[i + seg];
        }
        dp[i] = best;
    }

    int total = dp[0];
    free(dp); free(L);
    return total;
}

int gene_detect(const SuffixAutomaton* sam, const char* gene, int tamSubcadeia) {
    int total = maxMatchDP(sam, gene, tamSubcadeia);
    return total >= (int)ceil(0.9 * strlen(gene));
}

typedef struct {
    Doenca* doencas;
    int start;
    int end;
    const char* dna;
    int tamSubcadeia;
} ThreadArgs;

void* thread_calc_prob(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;

    SuffixAutomaton* sam = buildSA(args->dna);
    for (int i = args->start; i < args->end; i++) {
        int detectadas = 0;
        for (int j = 0; j < args->doencas[i].num_genes; j++)
            if (gene_detect(sam, args->doencas[i].genes[j].sequencia, args->tamSubcadeia))
                detectadas++;
        int perc = round((double)detectadas / args->doencas[i].num_genes * 100);
        args->doencas[i].prob = perc >= 90 ? 100 : perc;
    }

    free(sam->sa);
    free(sam);

    return NULL;
}

void calc_prob(Doenca* doencas, int numDoencas, const char* dna, int tamSubcadeia) {
    int num_threads = 4; // ajuste conforme CPU
    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    int block = (numDoencas + num_threads - 1) / num_threads;

    for (int t = 0; t < num_threads; t++) {
        args[t].doencas = doencas;
        args[t].dna = dna;
        args[t].tamSubcadeia = tamSubcadeia;
        args[t].start = t * block;
        args[t].end = (t + 1) * block;
        if (args[t].end > numDoencas) args[t].end = numDoencas;

        pthread_create(&threads[t], NULL, thread_calc_prob, &args[t]);
    }

    for (int t = 0; t < num_threads; t++)
        pthread_join(threads[t], NULL);
}


void merge(Doenca* arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    Doenca* L = (Doenca*) malloc(sizeof(Doenca) * n1);
    Doenca* R = (Doenca*) malloc(sizeof(Doenca) * n2);

    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int i = 0; i < n2; i++) R[i] = arr[m + 1 + i];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i].prob > R[j].prob || (L[i].prob == R[j].prob && L[i].order <= R[j].order))
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void mergeSort(Doenca* arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l)/2;
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
        merge(arr, l, m, r);
    }
}

void print_doenca(FILE* output, Doenca* doencas, int numDoencas) {
    for (int i = 0; i < numDoencas; i++)
        fprintf(output, "%s->%d%%\n", doencas[i].nome, doencas[i].prob);
}

int main(int argc, char* argv[]) {
    clock_t start = clock();

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");

    char* dna;
    int tamSubcadeia, numDoencas;
    Doenca* doencas;

    extrair_info(input, &dna, &tamSubcadeia, &doencas, &numDoencas);
    fclose(input);

    calc_prob(doencas, numDoencas, dna, tamSubcadeia);
    mergeSort(doencas, 0, numDoencas-1);
    print_doenca(output, doencas, numDoencas);
    fclose(output);

    // Libera memória
    free(dna);
    for (int i = 0; i < numDoencas; i++) {
        free(doencas[i].nome);
        for (int j = 0; j < doencas[i].num_genes; j++)
            free(doencas[i].genes[j].sequencia);
        free(doencas[i].genes);
    }
    free(doencas);

    clock_t end = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    fprintf(stderr, "Tempo de execucao: %.0f ms\n", duration);

    return 0;
}
