#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Funções auxiliares ---
static inline char *skip_ws(char *p) {
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') ++p;
    return p;
}

static inline int parse_int(char **pp) {
    char *p = skip_ws(*pp);
    int sign = 1, v = 0;
    if (*p == '-') { sign = -1; ++p; }
    while (*p >= '0' && *p <= '9') { v = v * 10 + (*p++ - '0'); }
    *pp = p;
    return v * sign;
}

static inline char *parse_token_and_null(char **pp) {
    char *p = skip_ws(*pp);
    char *start = p;
    while (*p && *p != ' ' && *p != '\n' && *p != '\r' && *p != '\t') ++p;
    if (*p) *p++ = '\0';
    *pp = p;
    return start;
}

// --- Heapsort ---
static inline int left_i(int i) { return (i << 1) + 1; }
static inline int right_i(int i){ return (i << 1) + 2; }

static inline void heapify(char **A, int n, int i) {
    while (1) {
        int largest = i;
        int l = left_i(i);
        int r = right_i(i);
        if (l < n && strcmp(A[l], A[largest]) > 0) largest = l;
        if (r < n && strcmp(A[r], A[largest]) > 0) largest = r;
        if (largest == i) return;
        char *t = A[i]; A[i] = A[largest]; A[largest] = t;
        i = largest;
    }
}

static inline void heapsort(char **A, int n) {
    if (n <= 1) return;
    for (int i = (n >> 1) - 1; i >= 0; --i) heapify(A, n, i);
    for (int i = n - 1; i > 0; --i) {
        char *t = A[0]; A[0] = A[i]; A[i] = t;
        heapify(A, i, 0);
    }
}

// --- Inserção ordenada de prioridade ---
static inline int priority_index(const int *arr, int count, int key) {
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
    const int total = parse_int(&p);
    const int limit = parse_int(&p);

    long outpool_cap = fsz * 2 + 1024;
    char *outpool = (char*) malloc(outpool_cap);
    long outpool_pos = 0;

    long filebuf_cap = outpool_cap;
    char *filebuf = (char*) malloc(filebuf_cap);
    long filebuf_pos = 0;

    int cap = 16, count = 0, limit_counter = 0;
    int *priorities = (int*) malloc(sizeof(int) * cap);
    char **pkt_ptrs = (char**) malloc(sizeof(char*) * cap);
    int *pkt_lens = (int*) malloc(sizeof(int) * cap);
    char **tokens = NULL;
    int tokens_cap = 0;

    for (int i = 0; i < total; ++i) {
        int priority = parse_int(&p);
        int size = parse_int(&p);
        if (size > tokens_cap) {
            tokens_cap = size << 1;
            tokens = (char**) realloc(tokens, sizeof(char*) * tokens_cap);
        }

        for (int j = 0; j < size; ++j)
            tokens[j] = parse_token_and_null(&p);

        heapsort(tokens, size);

        const long start = outpool_pos;
        outpool[outpool_pos++] = '|';
        for (int j = 0; j < size; ++j) {
            char *t = tokens[j];
            const size_t L = strlen(t);
            if (outpool_pos + L + 2 >= outpool_cap) {
                outpool_cap = (outpool_cap << 1) + L + 1024;
                outpool = (char*) realloc(outpool, outpool_cap);
            }
            memcpy(outpool + outpool_pos, t, L);
            outpool_pos += L;
            if (j != size - 1) outpool[outpool_pos++] = ',';
        }
        const long len = outpool_pos - start;

        if (limit_counter + size > limit && count) {
            for (int q = 0; q < count; ++q) {
                int lenq = pkt_lens[q];
                if (filebuf_pos + lenq + 2 >= filebuf_cap) {
                    filebuf_cap = (filebuf_cap << 1) + lenq + 1024;
                    filebuf = (char*) realloc(filebuf, filebuf_cap);
                }
                memcpy(filebuf + filebuf_pos, pkt_ptrs[q], lenq);
                filebuf_pos += lenq;
            }
            filebuf[filebuf_pos++] = '|';
            filebuf[filebuf_pos++] = '\n';
            count = 0;
            limit_counter = 0;
        }

        if (count == cap) {
            cap <<= 1;
            priorities = (int*) realloc(priorities, sizeof(int) * cap);
            pkt_ptrs = (char**) realloc(pkt_ptrs, sizeof(char*) * cap);
            pkt_lens = (int*) realloc(pkt_lens, sizeof(int) * cap);
        }

        int idx = priority_index(priorities, count, priority);
        if (count > idx) {
            memmove(priorities + idx + 1, priorities + idx, sizeof(int) * (count - idx));
            memmove(pkt_ptrs + idx + 1, pkt_ptrs + idx, sizeof(char*) * (count - idx));
            memmove(pkt_lens + idx + 1, pkt_lens + idx, sizeof(int) * (count - idx));
        }

        priorities[idx] = priority;
        pkt_ptrs[idx] = outpool + start;
        pkt_lens[idx] = (int) len;
        count++;
        limit_counter += size;
    }

    if (count) {
        for (int q = 0; q < count; ++q) {
            int lenq = pkt_lens[q];
            if (filebuf_pos + lenq + 1 >= filebuf_cap) {
                filebuf_cap = (filebuf_cap << 1) + lenq + 1024;
                filebuf = (char*) realloc(filebuf, filebuf_cap);
            }
            memcpy(filebuf + filebuf_pos, pkt_ptrs[q], lenq);
            filebuf_pos += lenq;
        }
        filebuf[filebuf_pos++] = '|';
    }

    if (filebuf_pos)
        fwrite(filebuf, 1, filebuf_pos, out);

    fclose(out);
    
    free(buf);
    free(outpool);
    free(filebuf);
    free(priorities);
    free(pkt_ptrs);
    free(pkt_lens);
    free(tokens);
    return 0;
}
