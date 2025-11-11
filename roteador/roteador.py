import sys
from time import perf_counter

def esquerdo(i: int) -> int:
    return 2 * i + 1

def direito(i: int) -> int:
    return 2 * i + 2

def trocar(V: list[int], i: int, j: int) -> None:
    V[i], V[j] = V[j], V[i]

def heapify(V: list[int], T: int, i: int) -> None:
    P = i
    E = esquerdo(i)
    D = direito(i)

    if E < T and V[E] > V[P]:
        P = E
    if D < T and V[D] > V[P]:
        P = D

    if P != i:
        trocar(V, P, i)
        heapify(V, T, P)

def construir_heap(V: list[int], n: int) -> None:
    for i in range(n // 2 - 1, -1, -1):
        heapify(V, n, i)

def heapsort(V: list[str]) -> list[str]:
    n = len(V)
    # se os elementos forem strings hexadecimais, podemos comparar diretamente como strings
    construir_heap(V, n)
    for i in range(n - 1, 0, -1):
        trocar(V, 0, i)
        heapify(V, i, 0)
    return V

def get_piority_index(priority_list: list, current_priority: int):
    for index, n in enumerate(priority_list):
        if current_priority > n:
            return index
    return len(priority_list)

def read_file(input_path):
    final_lines = []

    limit_counter = 0
    current_line = []
    current_priority = []

    with open(input_path, "r") as f:
        line0 = f.readline().split()
        _, limit = int(line0[0]), int(line0[1])

        for line in f:
            splited_line = line.split()

            priority = int(splited_line[0])
            listt = heapsort(splited_line[2:])
            size = len(listt)

            if limit_counter + size > limit:
                current_line[-1] = current_line[-1] + '|\n'
                final_lines.append("".join(current_line))
                current_line = []
                current_priority = []
                limit_counter = 0

            index = get_piority_index(current_priority, priority)
            current_priority.insert(index, priority)
            current_line.insert(index, f'|{",".join(listt)}')
            limit_counter += size

        if current_line:
            current_line[-1] = current_line[-1] + '|'
            final_lines.append("".join(current_line))

    return final_lines

def write_file(output_path, final_lines):
    with open(output_path, "w") as f:
        f.writelines(final_lines)

def main(args):
    start = perf_counter()
    final_lines = read_file(args[1])
    write_file(args[2], final_lines)
    end = perf_counter()
    print(f"Tempo de execução: {end - start:.6f}s")

if __name__ == "__main__":
    main(sys.argv)
