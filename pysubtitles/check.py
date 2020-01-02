#!/usr/bin/env python3

import sys
import getopt

# Global options with default values.
verbose = False


def main():
    args = parse_options(sys.argv[1:])

    if len(args) != 1:
        error("Invalid syntax")
        usage()
        sys.exit(2)

    filename = args[0]
    print("Checking file " + filename)


def usage():
    print("""Usage: check.py <filename>

    This performs subtitle checks on the given file.

    Options:
    -h              Show this help text.
    -v              Display verbose output.
    """)


def parse_options(args):

    # Access global variables
    global verbose

    # Parse options using Getopt; display an error and exit if options could not
    # be parsed.
    try:
        opts, args = getopt.getopt(args, "o:hv")
    except getopt.GetoptError as err:
        print(str(err))
        usage()
        sys.exit(2)

    # Set variables according to options
    for opt, val in opts:
        if opt == "-h":
            usage()
            sys.exit()
        elif opt == "-v":
            verbose = True
        else:
            assert False, "unhandled option"

    return args


def debug(s):
    if verbose:
        sys.stderr.write("DEBUG: " + s + "\n")

def error(s):
    sys.stderr.write("ERROR: " + s + "\n")


# This is the most important line: it calls the main function if this program is
# called directly.
if __name__ == "__main__":
    main()

