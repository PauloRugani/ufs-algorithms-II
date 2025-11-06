import sys

def main(args):
    print("#ARGS = %i"%len((args)))
    print("PROGRAMA = %s"%(args[0]))
    print("ARG1 = %s, ARG2 = %s" %(args[1], args[2]))

    golden_input = open(args[1], "r")
    golden_output = open(args[2], "w")
    #
    # ...
    #
    golden_input.close()
    golden_output.close()

if __name__ == "__main__":
    main(sys.argv)