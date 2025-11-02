# porto.py ./input/input.txt ./output/output.txt
# timeout: 1 second

import sys
sys.setrecursionlimit(100000000)

def merge_sort(list):
    if len(list) <= 1:
        return list

    half = len(list) // 2
    left = merge_sort(list[:half])
    right = merge_sort(list[half:])

    return merge(left, right)


def merge(left, right):
    if not left:
        return right
    if not right:
        return left

    if int(left[0].split('(')[1].split('%')[0]) >= int(right[0].split('(')[1].split('%')[0]):
        return [left[0]] + merge(left[1:], right)
    else:
        return [right[0]] + merge(left, right[1:])
    
def main(args):
    with open(args[1], "r") as file:
        n = file.readline()
        registered = {}
        registered_list = []
        for _ in range(0, int(n)):
            line = file.readline()
            registered_list.append(line[:11])
            registered[(line[:11])] = line.rstrip('\n')

        selected = {}
        m = file.readline()
        for _ in range(0, int(m)):
            line = file.readline()
            selected[(line[:11])] = line.rstrip('\n')

            
        cpf = []
        weight = []
    with open(args[2], "w") as file:
        for cod in registered_list:
            try:
                registered_cpf = registered[cod][12:30]
                selected_cpf = selected[cod][12:30]

                registered_weight = int(registered[cod][31:])
                selected_weight = int(selected[cod][31:])
                if registered_cpf != selected_cpf:
                    cpf.append(f'{cod}:{registered_cpf}<->{selected_cpf}\n')
                    continue

                dif = abs(selected_weight - registered_weight)
                perc = round((dif / registered_weight) * 100)
                if perc > 10:
                    weight.append(f'{cod}:{dif}kg({perc}%)\n')
            except Exception:
                ...

        file.writelines(cpf + merge_sort(weight))
        
if __name__ == "__main__":
    main(sys.argv)