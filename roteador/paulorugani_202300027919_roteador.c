#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Protótipos (necessário no MinGW) ---
int parse_int(char **pp);
char *skip_ws(char *p);
char *parse_token_and_null(char **pp);

// --- Funções auxiliares ---
char *skip_ws(char *p) {
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
    return p;
}

int parse_int(char **pp) {
    char *p = *pp;
    p = skip_ws(p);
    int sign = 1;
    if (*p == '-') { sign = -1; ++p; }
    int v = 0;
    while (*p >= '0' && *p <= '9') {
        v = v * 10 + (*p - '0');
        ++p;
    }
    *pp = p;
    return v * sign;
}

char *parse_token_and_null(char **pp) {
    char *p = *pp;
    p = skip_ws(p);
    char *start = p;
    while (*p != '\0' && *p != ' ' && *p != '\n' && *p != '\r' && *p != '\t') ++p;
    if (*p != '\0') { *p = '\0'; ++p; }
    *pp = p;
    return start;
}

// --- Heapsort e utilitários ---
static inline int left_i(int i) { return (i << 1) + 1; }
static inline int right_i(int i){ return (i << 1) + 2; }

void heapify(char **A, int n, int i) {
    while (1) {
        int largest = i;
        int l = left_i(i);
        int r = right_i(i);
        if (l < n && strcmp(A[l], A[largest]) > 0) largest = l;
        if (r < n && strcmp(A[r], A[largest]) > 0) largest = r;
        if (largest == i) break;
        char *t = A[i]; A[i] = A[largest]; A[largest] = t;
        i = largest;
    }
}

void heapsort(char **A, int n) {
    if (n <= 1) return;
    for (int i = (n >> 1) - 1; i >= 0; --i) heapify(A, n, i);
    for (int i = n - 1; i > 0; --i) {
        char *t = A[0]; A[0] = A[i]; A[i] = t;
        heapify(A, i, 0);
    }
}

int priority_index(const int *arr, int count, int key) {
    int lo = 0, hi = count;
    while (lo < hi) {
        int mid = (lo + hi) >> 1;
        if (key > arr[mid]) hi = mid;
        else lo = mid + 1;
    }
    return lo;
}

// --- Principal ---
int main(int argc, char **argv) {
    FILE *f = fopen(argv[1], "rb");
    FILE *out = fopen(argv[2], "wb");

    fseek(f, 0, SEEK_END);
    long fsz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char*) malloc(fsz + 2);
    fread(buf, 1, fsz, f);
    buf[fsz] = '\0';
    fclose(f);

    char *p = buf;
    int total = parse_int(&p);
    int limit = parse_int(&p);

    long out_pool_cap = fsz * 4 + 1024;
    char *outpool = (char*) malloc(out_pool_cap);
    long outpool_pos = 0;

    int cap = 16, count = 0, limit_counter = 0;
    int *priorities = (int*) malloc(sizeof(int) * cap);
    char **pkt_ptrs = (char**) malloc(sizeof(char*) * cap);
    int *pkt_lens = (int*) malloc(sizeof(int) * cap);
    char **tokens = NULL;

    for (int i = 0; i < total; ++i) {
        int priority = parse_int(&p);
        int size = parse_int(&p);
        tokens = (char**) malloc(sizeof(char*) * size);
        for (int j = 0; j < size; ++j)
            tokens[j] = parse_token_and_null(&p);

        heapsort(tokens, size);

        long start = outpool_pos;
        outpool[outpool_pos++] = '|';
        for (int j = 0; j < size; ++j) {
            char *toks = tokens[j];
            size_t L = strlen(toks);
            if (outpool_pos + L + 2 >= out_pool_cap) {
                out_pool_cap = out_pool_cap * 2 + L + 1024;
                outpool = (char*) realloc(outpool, out_pool_cap);
            }
            memcpy(outpool + outpool_pos, toks, L);
            outpool_pos += L;
            if (j != size - 1) outpool[outpool_pos++] = ',';
        }
        long len = outpool_pos - start;

        if (count == cap) {
            int newcap = cap << 1;
            priorities = (int*) realloc(priorities, sizeof(int) * newcap);
            pkt_ptrs = (char**) realloc(pkt_ptrs, sizeof(char*) * newcap);
            pkt_lens = (int*) realloc(pkt_lens, sizeof(int) * newcap);
            cap = newcap;
        }

        if (limit_counter + size > limit && count > 0) {
            for (int q = 0; q < count; ++q)
                fwrite(pkt_ptrs[q], 1, pkt_lens[q], out);
            fwrite("|\n", 1, 2, out);
            count = 0;
            limit_counter = 0;
        }

        int idx = priority_index(priorities, count, priority);
        for (int s = count; s > idx; --s) {
            priorities[s] = priorities[s-1];
            pkt_ptrs[s] = pkt_ptrs[s-1];
            pkt_lens[s] = pkt_lens[s-1];
        }

        priorities[idx] = priority;
        pkt_ptrs[idx] = outpool + start;
        pkt_lens[idx] = (int) len;

        count++;
        limit_counter += size;
        free(tokens);
    }

    if (count > 0) {
        for (int q = 0; q < count; ++q)
            fwrite(pkt_ptrs[q], 1, pkt_lens[q], out);
        fwrite("|", 1, 1, out);
    }

    fclose(out);
    return 0;
}
