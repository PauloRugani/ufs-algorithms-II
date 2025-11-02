# System lib
import sys

# Main function 
def main(args):
    print("#ARGS = %i"%len((args)))
    print("PROGRAMA = %s"%(args[0]))
    print("ARG1 = %s, ARG2 = %s" %(args[1], args[2]))

    # open files
    golden_input = open(args[1], "r")
    golden_output = open(args[2], "w")
    #
    # ...
    #
    # close files
    golden_input.close()
    golden_output.close()

# Entry point
if __name__ == "__main__":
    # Command line arguments
    main(sys.argv)