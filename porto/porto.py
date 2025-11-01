# porto.py ./input/input.txt ./output/output.txt
import sys

def main(args):

    with open(args[1], "r") as file:
        data = file.read()
    print(data)



    with open(args[2], "w") as file:
        file.write('exemplo')

if __name__ == "__main__":
    main(sys.argv)