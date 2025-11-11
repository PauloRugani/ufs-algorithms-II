#include <iostream>
#include <fstream>

using namespace std;

// ---------- Funções utilitárias ----------
inline int esquerdo(int i) { return (i << 1) + 1; }
inline int direito(int i)  { return (i << 1) + 2; }

inline void swap_str(string &a, string &b) {
    string tmp = static_cast<string&&>(a);
    a = static_cast<string&&>(b);
    b = static_cast<string&&>(tmp);
}

// ---------- Heapsort otimizado ----------
void heapify_str(string *V, int n, int i) {
    while (true) {
        int maior = i;
        int l = esquerdo(i);
        int r = direito(i);
        if (l < n && V[l] > V[maior]) maior = l;
        if (r < n && V[r] > V[maior]) maior = r;
        if (maior == i) break;
        swap_str(V[i], V[maior]);
        i = maior;
    }
}

void heapsort_str(string *V, int n) {
    for (int i = (n >> 1) - 1; i >= 0; --i)
        heapify_str(V, n, i);
    for (int i = n - 1; i > 0; --i) {
        swap_str(V[0], V[i]);
        heapify_str(V, i, 0);
    }
}

// ---------- Busca binária para prioridade ----------
inline int get_priority_index(const int *priorities, int count, int priority) {
    int ini = 0, fim = count;
    while (ini < fim) {
        int meio = (ini + fim) >> 1;
        if (priority > priorities[meio])
            fim = meio;
        else
            ini = meio + 1;
    }
    return ini;
}

// ---------- Programa principal ----------
int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    clock_t tstart = clock();

    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <arquivo_entrada> <arquivo_saida>\n";
        return 1;
    }

    ifstream fin(argv[1], ios::in);
    ofstream fout(argv[2], ios::out);
    if (!fin || !fout) {
        cerr << "Erro ao abrir arquivos.\n";
        return 1;
    }

    int totalPacotes = 0, limite = 0;
    fin >> totalPacotes >> limite;

    // Buffers fixos com expansão manual (para evitar realocações STL)
    int cap = 16;
    int *priorities = new int[cap];
    string *packets = new string[cap];
    int count = 0, limit_counter = 0;

    // Reutilização de buffers para evitar alocação repetida
    string token;
    string pkt;
    pkt.reserve(256);

    for (int p = 0; p < totalPacotes; ++p) {
        int priority, size;
        fin >> priority >> size;
        if (!fin) break;

        // Leitura e ordenação in-place
        string *elems = new string[size];
        for (int i = 0; i < size; ++i)
            fin >> elems[i];

        heapsort_str(elems, size);

        // Monta o pacote direto (sem stringstream, sem join)
        pkt.clear();
        pkt.push_back('|');
        for (int i = 0; i < size; ++i) {
            pkt += elems[i];
            if (i != size - 1) pkt.push_back(',');
        }
        delete[] elems;

        // Se excede limite, escreve o bloco atual e reseta
        if (limit_counter + size > limite && count > 0) {
            for (int i = 0; i < count; ++i)
                fout << packets[i];
            fout << "|\n";
            count = 0;
            limit_counter = 0;
        }

        // Inserção por busca binária (ordem decrescente)
        int idx = get_priority_index(priorities, count, priority);
        if (count == cap) {
            int newcap = cap << 1;
            int *npr = new int[newcap];
            string *npk = new string[newcap];
            for (int i = 0; i < count; ++i) {
                npr[i] = priorities[i];
                npk[i].swap(packets[i]);
            }
            delete[] priorities;
            delete[] packets;
            priorities = npr;
            packets = npk;
            cap = newcap;
        }

        for (int j = count; j > idx; --j) {
            priorities[j] = priorities[j - 1];
            packets[j].swap(packets[j - 1]);
        }

        priorities[idx] = priority;
        packets[idx].swap(pkt);

        ++count;
        limit_counter += size;
    }

    // Último flush
    if (count > 0) {
        for (int i = 0; i < count; ++i)
            fout << packets[i];
        fout << "|";
    }

    fin.close();
    fout.close();

    clock_t tend = clock();
    double elapsed = double(tend - tstart) / CLOCKS_PER_SEC;
    cout.setf(ios::fixed);
    cout.precision(6);
    cout << "Tempo de execução: " << elapsed << "s\n";

    return 0;
}
