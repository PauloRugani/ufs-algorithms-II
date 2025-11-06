#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>

using namespace std;

struct Counter {
    const char* name;
    int value;
};

struct Swap {
    int length;
    int *elements_sl;
    int *elements_ml;
    int *elements_rl;
    int *elements_sh;
    int *elements_mh;
    int *elements_rh;
    Counter counters[6];
};

Swap make_swap(int len, int *base_elements) {
    Swap s;
    s.length = len;
    s.elements_sl = new int[len];
    s.elements_ml = new int[len];
    s.elements_rl = new int[len];
    s.elements_sh = new int[len];
    s.elements_mh = new int[len];
    s.elements_rh = new int[len];
    for (int i = 0; i < len; ++i) {
        int v = base_elements[i];
        s.elements_sl[i] = v;
        s.elements_ml[i] = v;
        s.elements_rl[i] = v;
        s.elements_sh[i] = v;
        s.elements_mh[i] = v;
        s.elements_rh[i] = v;
    }
    s.counters[0] = {"LP", 0};
    s.counters[1] = {"LM", 0};
    s.counters[2] = {"LA", 0};
    s.counters[3] = {"HP", 0};
    s.counters[4] = {"HM", 0};
    s.counters[5] = {"HA", 0};
    return s;
}

void free_swap(Swap &s) {
    delete[] s.elements_sl;
    delete[] s.elements_ml;
    delete[] s.elements_rl;
    delete[] s.elements_sh;
    delete[] s.elements_mh;
    delete[] s.elements_rh;
}

inline void incCounter(Swap &list, const char *modo) {
    for (int i = 0; i < 6; ++i) {
        if (strcmp(list.counters[i].name, modo) == 0) {
            list.counters[i].value++;
            return;
        }
    }
}

inline int median_of_3(int *elements, int start, int end) {
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

// lomuto partition (elements é int*)
int lomuto(int *elements, int start, int end, Swap &list, const char *modo) {
    if (strcmp(modo, "LM") == 0) {
        int pivo_idx = median_of_3(elements, start, end);
        // swap e contador
        int tmp = elements[pivo_idx]; elements[pivo_idx] = elements[end]; elements[end] = tmp;
        incCounter(list, modo);
    } else if (strcmp(modo, "LA") == 0) {
        int n = end - start + 1;
        int val = elements[start];
        int aval = val < 0 ? -val : val;
        int pivo_idx = start + (aval % n);
        int tmp = elements[pivo_idx]; elements[pivo_idx] = elements[end]; elements[end] = tmp;
        incCounter(list, modo);
    }

    int pivo = elements[end];
    int x = start - 1;
    for (int y = start; y < end; ++y) {
        if (elements[y] <= pivo) {
            x++;
            int tmp = elements[x]; elements[x] = elements[y]; elements[y] = tmp;
            incCounter(list, modo);
        }
    }
    int tmp = elements[x+1]; elements[x+1] = elements[end]; elements[end] = tmp;
    incCounter(list, modo);
    return x + 1;
}

void lomuto_quicksort(int *elements, int start, int end, Swap &list, const char *modo) {
    incCounter(list, modo);
    if (start < end) {
        int p = lomuto(elements, start, end, list, modo);
        lomuto_quicksort(elements, start, p - 1, list, modo);
        lomuto_quicksort(elements, p + 1, end, list, modo);
    }
}

int hoare_partition(int *elements, int start, int end, Swap &list, const char *modo) {
    if (strcmp(modo, "HM") == 0) {
        int pivo_idx = median_of_3(elements, start, end);
        int tmp = elements[pivo_idx]; elements[pivo_idx] = elements[start]; elements[start] = tmp;
        incCounter(list, modo);
    } else if (strcmp(modo, "HA") == 0) {
        int n = end - start + 1;
        int val = elements[start];
        int aval = val < 0 ? -val : val;
        int pivo_idx = start + (aval % n);
        int tmp = elements[pivo_idx]; elements[pivo_idx] = elements[start]; elements[start] = tmp;
        incCounter(list, modo);
    }

    int pivo = elements[start];
    int x = start - 1;
    int y = end + 1;
    while (true) {
        do { y--; } while (elements[y] > pivo);
        do { x++; } while (elements[x] < pivo);
        if (x < y) {
            int tmp = elements[x]; elements[x] = elements[y]; elements[y] = tmp;
            incCounter(list, modo);
        } else {
            return y;
        }
    }
}

void hoare_quicksort(int *elements, int start, int end, Swap &list, const char *modo) {
    incCounter(list, modo);
    if (start < end) {
        int p = hoare_partition(elements, start, end, list, modo);
        hoare_quicksort(elements, start, p, list, modo);
        hoare_quicksort(elements, p + 1, end, list, modo);
    }
}

void sort_counters_stable(Counter arr[6]) {
    for (int i = 1; i < 6; ++i) {
        Counter key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].value > key.value) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

Swap* read_file(const char *input_path, int &out_count) {
    ifstream in(input_path);
    if (!in.is_open()) {
        cerr << "Erro ao abrir arquivo de entrada\n";
        out_count = 0;
        return nullptr;
    }
    int num_lists;
    in >> num_lists;
    out_count = num_lists;
    Swap *lists = new Swap[num_lists];
    for (int t = 0; t < num_lists; ++t) {
        int length;
        in >> length;
        int *temp = new int[length];
        for (int i = 0; i < length; ++i) in >> temp[i];
        // constrói Swap na posição t
        lists[t] = make_swap(length, temp);
        delete[] temp;
    }
    in.close();
    return lists;
}

void write_file(const char *output_path, Swap *lists, int count) {
    ofstream out(output_path);
    if (!out.is_open()) {
        cerr << "Erro ao abrir arquivo de saida\n";
        return;
    }
    for (int i = 0; i < count; ++i) {
        // cópia dos counters para ordenar
        Counter cpy[6];
        for (int k = 0; k < 6; ++k) cpy[k] = lists[i].counters[k];
        sort_counters_stable(cpy);
        out << "[" << lists[i].length << "]:";
        for (int k = 0; k < 6; ++k) {
            out << cpy[k].name << "(" << cpy[k].value << ")";
            if (k != 5) out << ",";
        }
        out << "\n";
    }
    out.close();
}

void process_list(Swap &list) {
    lomuto_quicksort(list.elements_sl, 0, list.length - 1, list, "LP");
    lomuto_quicksort(list.elements_ml, 0, list.length - 1, list, "LM");
    lomuto_quicksort(list.elements_rl, 0, list.length - 1, list, "LA");
    hoare_quicksort(list.elements_sh, 0, list.length - 1, list, "HP");
    hoare_quicksort(list.elements_mh, 0, list.length - 1, list, "HM");
    hoare_quicksort(list.elements_rh, 0, list.length - 1, list, "HA");
}

int main(int argc, char* argv[]) {
    int count;
    Swap *lists = read_file(argv[1], count);
    if (!lists) return 1;

    for (int i = 0; i < count; ++i) {
        process_list(lists[i]);
    }

    write_file(argv[2], lists, count);

    for (int i = 0; i < count; ++i) free_swap(lists[i]);
    delete[] lists;
    return 0;
}
