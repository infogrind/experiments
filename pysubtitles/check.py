#!/usr/bin/env python3

import sys
import getopt
import logging
import color_logs
import os.path

# Global options object
options = {}

# Global logger
log = logging.getLogger(__name__)

def main():
    args = parse_options(sys.argv[1:])
    setupLogging()

    log.info(f'Starting {os.path.basename(__file__)}')

    files = args
    if not files:
        log.warning("No files specified; I will do nothing.")

    for f in files:
        checkFile(f)


def checkFile(f):
    log.info(f'Checking file {f}')
    if not os.path.exists(f):
        log.warning(f'File {f} does not exist, skipping')
        return
    log.debug(f'File {f} exists, continuing.')



def setupLogging():
    formatter = logging.Formatter(
            '%(asctime)s %(levelname)s %(message)s', '%m-%d %H:%M:%S')
    handler = logging.StreamHandler(sys.stderr)
    handler.setFormatter(formatter)

    if 'verbose' in options:
        level = logging.DEBUG
    elif 'quiet' in options:
        level = logging.WARN
    else:
        level = logging.INFO

    logger = logging.getLogger(__name__)
    logger.setLevel(level)
    logger.addHandler(handler)


def usage():
    print("""Usage: check.py <files...>

    This performs subtitle checks on the given files.

    Options:
    -h              Show this help text.
    -q              Quiet mode (show only warnings and errors)
    -v              Display verbose output.
    """)


def parse_options(args):

    global options

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
            options['verbose'] = 1
        elif opt == "-q":
            options['quiet'] = 1
        else:
            assert False, "unhandled option"

    return args


def error(s):
    sys.stderr.write(s + "\n")


# This is the most important line: it calls the main function if this program is
# called directly.
if __name__ == "__main__":
    main()

