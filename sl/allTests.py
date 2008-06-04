#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re, getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

def isCygwin():

    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def isWin32():

    return sys.platform == "win32" or isCygwin()

def isWin9x():

    if isWin32():
        return not (os.environ.has_key("OS") and os.environ["OS"] == "Windows_NT")
    else:
        return 0

def isVista():
    return isWin32() and sys.getwindowsversion()[0] == 6

def runTests(args, tests, num = 0):
    #
    # Run each of the tests.
    #
    for i in tests:

        i = os.path.normpath(i)
        dir = os.path.join(toplevel, "test", i)

        print
        if num > 0:
            print "[" + str(num) + "]",
        print "*** running tests in " + dir,
        print

        if isWin9x():
            status = os.system("python " + os.path.join(dir, "run.py " + args))
        else:
            status = os.system(os.path.join(dir, "run.py " + args))

        if status:
            if(num > 0):
                print "[" + str(num) + "]",
            print "test in " + dir + " failed with exit status", status,
            sys.exit(status)

#
# List of all basic tests.
#
tests = [ \
    "IceCS/proxy", \
    "IceCS/operations", \
    "IceCS/exceptions", \
    "IceCS/inheritance", \
    "IceCS/facets", \
    "IceCS/objects", \
    "IceCS/slicing/exceptions", \
    "IceCS/slicing/objects", \
    "IceCS/retry", \
    "IceCS/dictMapping", \
    "IceCS/seqMapping", \
    ]

def usage():
    print "usage: %s " % (sys.argv[0])
    print "  --mono                  Run the tests with mono."
    print "  --start=<regex>         Start running the tests at the given test."
    print "  --start-after=<regex>   Start running the tests after the given test."
    print "  --loop                  Run the tests in a loop."
    print "  --filter=<regex>        Run all the tests that match the given regex."
    print "  --rfilter=<regex>       Run all the tests that do not match the given regex."
    print "  --debug                 Display debugging information on each test."
    print "  --host=host             Set --Ice.Default.Host=<host>."
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "mlr:R:",
        ["start=", "start-after=", "filter=", "rfilter=", "loop", "debug", "host=", "mono"])
except getopt.GetoptError:
    usage()

# Extra args cause a usage error.
if args:
    usage()

def index(l, re):
    """Find the index of the first item in the list that matches the given re"""
    for i in range(0, len(l)):
        if re.search(l[i]):
            return i
    return -1

# Process the command line args.
mono = False
loop = False
arg = ""
for o, a in opts:
    if o in ("-l", "--loop"):
        loop = True
    elif o in ("-r", "-R", "--filter", '--rfilter'):
        regexp = re.compile(a)
        if o in ("--rfilter", "-R"):
            tests = [ x for x in tests if not regexp.search(x) ]
        else:
            tests = [ x for x in tests if regexp.search(x) ]
    elif o in ( "--host", "--debug" ):
        arg += " " + o
        if len(a) > 0:
            arg += " " + a
    elif o in ('--start', "--start-after"):
        start = index(tests, re.compile(a))
        if start == -1:
            print "test %s not found. no tests to run" % (a)
            sys.exit(2)
        if o == "--start-after":
            start += 1
        tests = tests[start:]
    elif o in ( "-m", "--mono" ):
        args += " " + o
        mono = True

args = [ arg ]

if not isWin32():
    mono = True

# For mono or vista the IceSSL configuration test is removed.
if mono or isVista():
    try:
	tests.remove("IceSSL/configuration")
    except ValueError:
	pass

# Run the tests.
if loop:
    num = 1
    while 1:
        for arg in args:
            runTests(arg, tests, num)
        num += 1
else:
    for arg in args:
        runTests(arg, tests)
