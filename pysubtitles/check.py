#!/usr/bin/env python3

import sys
import getopt
import logging


def main():
    args = parse_options(sys.argv[1:])

    # Set default logging level (this has no effect if logging.basicConfig has
    # already been called as part of option parsing).
    logging.basicConfig(level=logging.INFO)

    logging.debug('This is a debug message')
    logging.info('This is an info message')
    logging.warning('This is a warning message')
    logging.error('This is an error message')
    logging.critical('This is a critical message')

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
    -q              Quiet mode (show only warnings and errors)
    -v              Display verbose output.
    """)


def parse_options(args):

    # Parse options using Getopt; display an error and exit if options could not
    # be parsed.
    try:
        opts, args = getopt.getopt(args, "o:hvq")
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
            logging.basicConfig(level=logging.DEBUG)
        elif opt == "-q":
            logging.basicConfig(level=logging.WARN)
        else:
            assert False, "unhandled option"

    return args


def error(s):
    sys.stderr.write(s + "\n")


# This is the most important line: it calls the main function if this program is
# called directly.
if __name__ == "__main__":
    main()

