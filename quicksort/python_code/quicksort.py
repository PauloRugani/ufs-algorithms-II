# quicksort.py ./input/input.txt ./output/output.txt
import sys
import math
from time import perf_counter
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor

sys.setrecursionlimit(100000000)


class Swap:
    def __init__(self, length, elements):
        self.length = length
        self.elements_sl = elements[:]
        self.elements_ml = elements[:]
        self.elements_rl = elements[:]
        self.elements_sh = elements[:]
        self.elements_mh = elements[:]
        self.elements_rh = elements[:]
        self.counters = {
            "LP": 0,
            "LM": 0,
            "LA": 0,
            "HP": 0,
            "HM": 0,
            "HA": 0
        }



def switch(a, b, list: Swap, modo: str):
    a[0], b[0] = b[0], a[0]
    list.counters[modo] += 1


def median_of_3(elements, start, end):
    n = end - start + 1
    idx1 = start + n // 4
    idx2 = start + n // 2
    idx3 = start + (3 * n) // 4

    v1, v2, v3 = elements[idx1], elements[idx2], elements[idx3]
    if (v1 >= v2 and v1 <= v3) or (v1 <= v2 and v1 >= v3):
        return idx1
    elif (v2 >= v1 and v2 <= v3) or (v2 <= v1 and v2 >= v3):
        return idx2
    else:
        return idx3


def lomuto(elements, start, end, list: Swap, modo: str):
    if modo == "LM":
        pivo_idx = median_of_3(elements, start, end)
        switch([elements[pivo_idx]], [elements[end]], list, modo)
        elements[pivo_idx], elements[end] = elements[end], elements[pivo_idx]
    elif modo == "LA":
        n = end - start + 1
        pivo_idx = start + abs(elements[start]) % n
        switch([elements[pivo_idx]], [elements[end]], list, modo)
        elements[pivo_idx], elements[end] = elements[end], elements[pivo_idx]

    pivo = elements[end]
    x = start - 1
    for y in range(start, end):
        if elements[y] <= pivo:
            x += 1
            switch([elements[x]], [elements[y]], list, modo)
            elements[x], elements[y] = elements[y], elements[x]
    switch([elements[x + 1]], [elements[end]], list, modo)
    elements[x + 1], elements[end] = elements[end], elements[x + 1]
    return x + 1


def lomuto_quicksort(elements, start, end, list: Swap, modo: str):
    list.counters[modo] += 1
    if start < end:
        pivo = lomuto(elements, start, end, list, modo)
        lomuto_quicksort(elements, start, pivo - 1, list, modo)
        lomuto_quicksort(elements, pivo + 1, end, list, modo)


def hoare(elements, start, end, list: Swap, modo: str):
    if modo == "HM":
        pivo_idx = median_of_3(elements, start, end)
        switch([elements[pivo_idx]], [elements[start]], list, modo)
        elements[pivo_idx], elements[start] = elements[start], elements[pivo_idx]
    elif modo == "HA":
        n = end - start + 1
        pivo_idx = start + abs(elements[start]) % n
        switch([elements[pivo_idx]], [elements[start]], list, modo)
        elements[pivo_idx], elements[start] = elements[start], elements[pivo_idx]

    pivo = elements[start]
    x = start - 1
    y = end + 1
    while True:
        y -= 1
        while elements[y] > pivo:
            y -= 1
        x += 1
        while elements[x] < pivo:
            x += 1
        if x < y:
            switch([elements[x]], [elements[y]], list, modo)
            elements[x], elements[y] = elements[y], elements[x]
        else:
            return y


def hoare_quicksort(elements, start, end, list: Swap, modo: str):
    list.counters[modo] += 1
    if start < end:
        pivo = hoare(elements, start, end, list, modo)
        hoare_quicksort(elements, start, pivo, list, modo)
        hoare_quicksort(elements, pivo + 1, end, list, modo)

def merge_sort(counters):
    if len(counters) > 1:
        meio = len(counters) // 2
        esquerda = counters[:meio]
        direita = counters[meio:]

        merge_sort(esquerda)
        merge_sort(direita)

        i = j = k = 0
        while i < len(esquerda) and j < len(direita):
            if esquerda[i][1] <= direita[j][1]:
                counters[k] = esquerda[i]
                i += 1
            else:
                counters[k] = direita[j]
                j += 1
            k += 1
        while i < len(esquerda):
            counters[k] = esquerda[i]
            i += 1
            k += 1
        while j < len(direita):
            counters[k] = direita[j]
            j += 1
            k += 1

def read_file(input_path):
    lists = []
    with open(input_path, "r") as f:
        num_lists = int(f.readline().strip())
        for _ in range(num_lists):
            length = int(f.readline().strip())
            elements = list(map(int, f.readline().strip().split()))
            lists.append(Swap(length, elements))
    return lists

def write_file(output_path, lists):
    with open(output_path, "w") as f:
        for ilist in lists:
            conts = list(ilist.counters.items())
            merge_sort(conts)
            line = f"[{ilist.length}]:"
            line += ",".join([f"{nome}({valor})" for nome, valor in conts])
            f.write(line + "\n")

def process_list(list):
    with ThreadPoolExecutor(max_workers=6) as executor:
        futures = [
            executor.submit(lomuto_quicksort, list.elements_sl, 0, list.length - 1, list, "LP"),
            executor.submit(lomuto_quicksort, list.elements_ml, 0, list.length - 1, list, "LM"),
            executor.submit(lomuto_quicksort, list.elements_rl, 0, list.length - 1, list, "LA"),
            executor.submit(hoare_quicksort, list.elements_sh, 0, list.length - 1, list, "HP"),
            executor.submit(hoare_quicksort, list.elements_mh, 0, list.length - 1, list, "HM"),
            executor.submit(hoare_quicksort, list.elements_rh, 0, list.length - 1, list, "HA"),
        ]
        for f in futures:
            f.result() 
    return list


def main(args):
    start = perf_counter()
    lists = read_file(args[1])

    with ProcessPoolExecutor() as executor:
        results = list(executor.map(process_list, lists))

    write_file(args[2], results)
    end = perf_counter()

    print(f"Tempo total: {end - start:.6f} segundos")
if __name__ == "__main__":
    main(sys.argv)
