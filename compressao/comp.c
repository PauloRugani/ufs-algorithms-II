#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_SEQ 10000
#define MAX_NODES 512

int rle_compress(unsigned char *data, int n, unsigned char *out) {
    int i = 0, k = 0;
    while (i < n) {
        int run = 1;
        while (i + run < n && data[i] == data[i + run] && run < 255)
            run++;
        out[k++] = (unsigned char)run;
        out[k++] = data[i];
        i += run;
    }
    return k;
}

typedef struct HuffNode {
    int freq;
    int symbol;
    int minLeaf;
    struct HuffNode *left;
    struct HuffNode *right;
} HuffNode;

static int heapLess(HuffNode *a, HuffNode *b) {
    if (a->freq != b->freq) return a->freq < b->freq;
    if (a->symbol != -1 && b->symbol != -1) return a->symbol < b->symbol;
    return a->minLeaf < b->minLeaf;
}

typedef struct {
    HuffNode* arr[MAX_NODES];
    int size;
} Heap;

static void heapPush(Heap *heap, HuffNode *node) {
    int i = heap->size++;
    heap->arr[i] = node;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (heapLess(heap->arr[i], heap->arr[p])) {
            HuffNode *t = heap->arr[i]; heap->arr[i] = heap->arr[p]; heap->arr[p] = t;
            i = p;
        } else break;
    }
}

static HuffNode* heapPop(Heap *heap) {
    HuffNode *ret = heap->arr[0];
    heap->arr[0] = heap->arr[--heap->size];
    int i = 0;
    while (1) {
        int l = 2*i+1, r = 2*i+2, s = i;
        if (l < heap->size && heapLess(heap->arr[l], heap->arr[s])) s = l;
        if (r < heap->size && heapLess(heap->arr[r], heap->arr[s])) s = r;
        if (s != i) { HuffNode *t = heap->arr[i]; heap->arr[i] = heap->arr[s]; heap->arr[s] = t; i = s; }
        else break;
    }
    return ret;
}

static HuffNode* buildHuffman(int *freq, HuffNode *nodes, int *nodeCount) {
    Heap heap; heap.size = 0;
    *nodeCount = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            nodes[*nodeCount].freq = freq[i];
            nodes[*nodeCount].symbol = i;
            nodes[*nodeCount].minLeaf = i;
            nodes[*nodeCount].left = NULL;
            nodes[*nodeCount].right = NULL;
            heapPush(&heap, &nodes[(*nodeCount)++]);
        }
    }
    if (heap.size == 0) return NULL;
    if (heap.size == 1) return heapPop(&heap);
    while (heap.size > 1) {
        HuffNode *a = heapPop(&heap);
        HuffNode *b = heapPop(&heap);
        nodes[*nodeCount].freq = a->freq + b->freq;
        nodes[*nodeCount].symbol = -1;
        nodes[*nodeCount].minLeaf = a->minLeaf < b->minLeaf ? a->minLeaf : b->minLeaf;
        nodes[*nodeCount].left = a;
        nodes[*nodeCount].right = b;
        heapPush(&heap, &nodes[(*nodeCount)++]);
    }
    return heapPop(&heap);
}

typedef struct {
    unsigned int code;
    int len;
} HuffCode;

static void buildCodes(HuffNode *node, unsigned int code, int len, HuffCode codes[256]) {
    if (!node) return;
    if (!node->left && !node->right) {
        codes[node->symbol].code = code;
        codes[node->symbol].len = len ? len : 1;
        return;
    }
    if (node->left) buildCodes(node->left, (code<<1)|0u, len+1, codes);
    if (node->right) buildCodes(node->right, (code<<1)|1u, len+1, codes);
}

static int huffmanEncode(unsigned char *data, int n, unsigned char *out) {
    int freq[256] = {0};
    for (int i = 0; i < n; i++) freq[data[i]]++;

    HuffNode nodes[MAX_NODES];
    int nodeCount = 0;
    HuffNode *root = buildHuffman(freq, nodes, &nodeCount);
    if (!root) return 0;

    HuffCode codes[256];
    for (int i = 0; i < 256; i++) { codes[i].code = 0; codes[i].len = 0; }
    buildCodes(root, 0u, 0, codes);

    int outLen = 0;
    unsigned char byte = 0;
    int bitPos = 0;
    for (int i = 0; i < n; i++) {
        unsigned int c = codes[data[i]].code;
        int l = codes[data[i]].len;
        for (int b = l-1; b >= 0; b--) {
            byte = (unsigned char)((byte << 1) | ((c >> b) & 1u));
            bitPos++;
            if (bitPos == 8) { out[outLen++] = byte; byte = 0; bitPos = 0; }
        }
    }
    if (bitPos) out[outLen++] = (unsigned char)(byte << (8 - bitPos));
    return outLen;
}

static char hex_lookup[256][2];
static void init_hex_lookup(void) {
    const char *h = "0123456789ABCDEF";
    for (int i = 0; i < 256; ++i) {
        hex_lookup[i][0] = h[(i >> 4) & 0xF];
        hex_lookup[i][1] = h[i & 0xF];
    }
}

static void processSequence_to_buffer(unsigned char *data, int n, char *buf, size_t bufcap) {
    unsigned char rle[MAX_SEQ*2];
    int rle_sz = rle_compress(data, n, rle);

    unsigned char huf[MAX_SEQ];
    int huf_sz = huffmanEncode(data, n, huf);

    double rle_rate = (double)rle_sz / n * 100.0;
    double huf_rate = (double)huf_sz / n * 100.0;

    size_t pos = 0;
    if (huf_sz < rle_sz) {
        int written = snprintf(buf + pos, bufcap - pos, "HUF(%.2f%%)=", huf_rate);
        if (written > 0) pos += (size_t)written;
        for (int i = 0; i < huf_sz; i++) {
            if (pos + 2 >= bufcap) break;
            unsigned char v = huf[i];
            buf[pos++] = hex_lookup[v][0];
            buf[pos++] = hex_lookup[v][1];
        }
    } else {
        int written = snprintf(buf + pos, bufcap - pos, "RLE(%.2f%%)=", rle_rate);
        if (written > 0) pos += (size_t)written;
        for (int i = 0; i < rle_sz; i += 2) {
            if (pos + 4 >= bufcap) break;
            unsigned char a = rle[i];
            unsigned char b = rle[i+1];
            buf[pos++] = hex_lookup[a][0];
            buf[pos++] = hex_lookup[a][1];
            buf[pos++] = hex_lookup[b][0];
            buf[pos++] = hex_lookup[b][1];
        }
    }
    if (pos >= bufcap) buf[bufcap-1] = '\0'; else buf[pos] = '\0';
}

typedef struct {
    int T;
    unsigned char **seq_data;
    int *seq_n;
    char **outbufs;
    size_t *outcaps;
    int thread_id;
    int num_threads;
} PoolCtx;

static void *pool_thread(void *arg) {
    PoolCtx *ctx = (PoolCtx*)arg;
    int id = ctx->thread_id;
    int stride = ctx->num_threads;
    for (int t = id; t < ctx->T; t += stride) {
        processSequence_to_buffer(ctx->seq_data[t], ctx->seq_n[t], ctx->outbufs[t], ctx->outcaps[t]);
    }
    return NULL;
}

static int get_num_cores_env(void) {
    char *env = getenv("NUMBER_OF_PROCESSORS");
    if (env) {
        int v = atoi(env);
        if (v > 0) return v;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) { fprintf(stderr, "Uso: %s <entrada> <saida>\n", argv[0]); return 1; }

    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");
    if (!in || !out) { fprintf(stderr, "Erro ao abrir arquivos\n"); return 1; }

    init_hex_lookup();

    int T;
    if (fscanf(in, "%d", &T) != 1) { fclose(in); fclose(out); return 1; }
    if (T <= 0) { fclose(in); fclose(out); return 0; }

    unsigned char **seq_data = (unsigned char**)malloc(sizeof(unsigned char*) * (size_t)T);
    int *seq_n = (int*)malloc(sizeof(int) * (size_t)T);
    char **outbufs = (char**)malloc(sizeof(char*) * (size_t)T);
    size_t *outcaps = (size_t*)malloc(sizeof(size_t) * (size_t)T);
    if (!seq_data || !seq_n || !outbufs || !outcaps) {
        fprintf(stderr, "Memoria insuficiente\n");
        return 1;
    }

    for (int t = 0; t < T; t++) {
        int n;
        if (fscanf(in, "%d", &n) != 1) n = 0;
        if (n < 0) n = 0;
        if (n > MAX_SEQ) n = MAX_SEQ;
        seq_n[t] = n;
        seq_data[t] = (unsigned char*)malloc((size_t)n);
        if (!seq_data[t]) { fprintf(stderr, "Memoria insuficiente\n"); return 1; }
        for (int i = 0; i < n; i++) {
            unsigned int x;
            if (fscanf(in, "%x", &x) != 1) x = 0;
            seq_data[t][i] = (unsigned char)(x & 0xFFu);
        }
        size_t cap = (size_t)6 * (n > 1 ? n : 1) + 128;
        outcaps[t] = cap;
        outbufs[t] = (char*)malloc(cap);
        if (!outbufs[t]) { fprintf(stderr, "Memoria insuficiente\n"); return 1; }
        outbufs[t][0] = '\0';
    }

    int cores = get_num_cores_env();
    int num_threads = T < cores ? T : cores;
    if (num_threads <= 0) num_threads = 1;

    pthread_t *workers = (pthread_t*)malloc(sizeof(pthread_t) * (size_t)num_threads);
    PoolCtx *ctxs = (PoolCtx*)malloc(sizeof(PoolCtx) * (size_t)num_threads);
    if (!workers || !ctxs) { fprintf(stderr, "Memoria insuficiente\n"); return 1; }

    for (int i = 0; i < num_threads; ++i) {
        ctxs[i].T = T;
        ctxs[i].seq_data = seq_data;
        ctxs[i].seq_n = seq_n;
        ctxs[i].outbufs = outbufs;
        ctxs[i].outcaps = outcaps;
        ctxs[i].thread_id = i;
        ctxs[i].num_threads = num_threads;
        if (pthread_create(&workers[i], NULL, pool_thread, &ctxs[i]) != 0) {
            /* fallback: run in main thread for this worker */
            pool_thread(&ctxs[i]);
            workers[i] = 0;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        if (workers[i]) pthread_join(workers[i], NULL);
    }

    for (int t = 0; t < T; t++) {
        fprintf(out, "%d->%s\n", t, outbufs[t]);
    }
    return 0;
}