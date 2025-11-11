#include <iostream>
#include <fstream>

using namespace std;

// Counteres nameados
struct Counter {
    string name;
    int value;
};

// List contendo 6 vetores e os counters
struct List {
    int lenght;
    int* elementsLP;
    int* elementsLM;
    int* elementsLA;
    int* elementsHP;
    int* elementsHM;
    int* elementsHA;
    Counter counters[6];
};

// modes
enum Modo { LP=0, LM=1, LA=2, HP=3, HM=4, HA=5 };

inline void swap(int &a, int &b, List& list, int mode) {
    int tmp = a; a = b; b = tmp;
    list.counters[mode].value++;
}

// mediana de tres (índices relativos a start..end)
int medianOfThree(int* elements, int start, int end) {
    int n = end - start + 1;
    int idx1 = start + n / 4;
    int idx2 = start + n / 2;
    int idx3 = start + (3 * n) / 4;
    int v1 = elements[idx1];
    int v2 = elements[idx2];
    int v3 = elements[idx3];
    if ((v1 >= v2 && v1 <= v3) || (v1 <= v2 && v1 >= v3)) return idx1;
    if ((v2 >= v1 && v2 <= v3) || (v2 <= v1 && v2 >= v3)) return idx2;
    return idx3;
}

// Lomuto
int lomuto(int* elements, int start, int end, List& list, int mode) {
    int pivotIndex = end;
    if (mode == LM) {
        pivotIndex = medianOfThree(elements, start, end);
        swap(elements[pivotIndex], elements[end], list, mode);
    } else if (mode == LA) {
        int n = end - start + 1;
        int idx = start + ( (elements[start] < 0) ? -elements[start] : elements[start]) % n;
        pivotIndex = idx;
        swap(elements[pivotIndex], elements[end], list, mode);
    }
    int pivo = elements[end];
    int x = start - 1;
    for (int y = start; y < end; ++y) {
        if (elements[y] <= pivo) {
            ++x;
            swap(elements[x], elements[y], list, mode);
        }
    }
    swap(elements[x+1], elements[end], list, mode);
    return x + 1;
}

void quicksortLomuto(int* elements, int start, int end, List& list, int mode) {
    list.counters[mode].value++; // conta a chamada (igual ao Python)
    if (start < end) {
        int p = lomuto(elements, start, end, list, mode);
        quicksortLomuto(elements, start, p - 1, list, mode);
        quicksortLomuto(elements, p + 1, end, list, mode);
    }
}

// Hoare
int hoare(int* elements, int start, int end, List& list, int mode) {
    int pivotIndex = start;
    if (mode == HM) {
        pivotIndex = medianOfThree(elements, start, end);
        swap(elements[pivotIndex], elements[start], list, mode);
    } else if (mode == HA) {
        int n = end - start + 1;
        int idx = start + ( (elements[start] < 0) ? -elements[start] : elements[start]) % n;
        pivotIndex = idx;
        swap(elements[pivotIndex], elements[start], list, mode);
    }
    int pivo = elements[start];
    int i = start - 1;
    int j = end + 1;
    while (true) {
        do { --j; } while (elements[j] > pivo);
        do { ++i; } while (elements[i] < pivo);
        if (i < j) swap(elements[i], elements[j], list, mode);
        else return j;
    }
}

void quicksortHoare(int* elements, int start, int end, List& list, int mode) {
    list.counters[mode].value++; // conta a chamada
    if (start < end) {
        int p = hoare(elements, start, end, list, mode);
        quicksortHoare(elements, start, p, list, mode);
        quicksortHoare(elements, p + 1, end, list, mode);
    }
}

void sortCounters(Counter* v, int n) {
    for (int i = 1; i < n; ++i) {
        Counter key = v[i];
        int j = i - 1;
        while (j >= 0 && v[j].value > key.value) {
            v[j+1] = v[j];
            --j;
        }
        v[j+1] = key;
    }
}

void lerArquivo(ifstream& fin, List*& lists, int& numLists) {
    fin >> numLists;
    lists = new List[numLists];
    for (int i = 0; i < numLists; ++i) {
        int tam;
        fin >> tam;
        lists[i].lenght = tam;
        lists[i].elementsLP = new int[tam];
        lists[i].elementsLM = new int[tam];
        lists[i].elementsLA = new int[tam];
        lists[i].elementsHP = new int[tam];
        lists[i].elementsHM = new int[tam];
        lists[i].elementsHA = new int[tam];
        // inicializa names dos counters (mesma ordem do Python)
        lists[i].counters[0].name = "LP";
        lists[i].counters[1].name = "LM";
        lists[i].counters[2].name = "LA";
        lists[i].counters[3].name = "HP";
        lists[i].counters[4].name = "HM";
        lists[i].counters[5].name = "HA";
        for (int k = 0; k < 6; ++k) lists[i].counters[k].value = 0;
        // ler elements (pode estar em uma ou várias linhas)
        for (int j = 0; j < tam; ++j) {
            int val;
            fin >> val;
            lists[i].elementsLP[j] = val;
            lists[i].elementsLM[j] = val;
            lists[i].elementsLA[j] = val;
            lists[i].elementsHP[j] = val;
            lists[i].elementsHM[j] = val;
            lists[i].elementsHA[j] = val;
        }
    }
}

int main(int argc, char* argv[]) {
    ifstream fin;
    ofstream fout;
    fin.open(argv[1]);
    fout.open(argv[2]);
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    List* lists = nullptr;
    int numLists = 0;
    lerArquivo(fin, lists, numLists);

    // processa cada list sequencialmente (mesma lógica do Python, sem paralelismo)
    for (int i = 0; i < numLists; ++i) {
        int tam = lists[i].lenght;
        if (tam > 0) {
            quicksortLomuto(lists[i].elementsLP, 0, tam - 1, lists[i], LP);
            quicksortLomuto(lists[i].elementsLM, 0, tam - 1, lists[i], LM);
            quicksortLomuto(lists[i].elementsLA, 0, tam - 1, lists[i], LA);
            quicksortHoare(lists[i].elementsHP, 0, tam - 1, lists[i], HP);
            quicksortHoare(lists[i].elementsHM, 0, tam - 1, lists[i], HM);
            quicksortHoare(lists[i].elementsHA, 0, tam - 1, lists[i], HA);
        } else {
            for (int m = 0; m < 6; ++m) lists[i].counters[m].value++;
        }
        // ordenar os 6 counters por value (estável)
        sortCounters(lists[i].counters, 6);
    }

    // escrita no formato do script Python: "[<lenght>]:LP(val),LM(val),..."
    for (int i = 0; i < numLists; ++i) {
        fout << "[" << lists[i].lenght << "]:";
        for (int j = 0; j < 6; ++j) {
            fout << lists[i].counters[j].name << "(" << lists[i].counters[j].value << ")";
            if (j + 1 < 6) fout << ",";
        }
        fout << "\n";
    }
    fin.close();
    fout.close();
    return 0;
}
