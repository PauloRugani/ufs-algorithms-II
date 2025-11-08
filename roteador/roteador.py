import sys
from time import perf_counter

def read_file(input_path):
    final_lines = []
    current_line = []
    limit_counter = 0

    with open(input_path, "r") as f:
        line0 = f.readline().split()
        _, limit = int(line0[0]), int(line0[1]) #! pensar como eu vou guardar a prioridade da lista em relação a outra

        for line in f:
            splited_line = line.split()

            priority = splited_line[0]
            listt = splited_line[2:]
            
            size = len(listt)

            if limit_counter + size >= limit:
                final_lines.append(current_line)
                current_line = []
                limit_counter = 0

            current_line.append(listt)
            limit_counter += size

        if current_line:
            final_lines.append(current_line)

    return final_lines


def main(args):
    start = perf_counter()
    aa = read_file(args[1])
    # write_file(args[2])

    end = perf_counter()
    # print(aa)
    print(end - start)
if __name__ == "__main__":
    main(sys.argv)