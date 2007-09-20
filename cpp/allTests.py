#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
tests = [ 
    "IceUtil/thread", 
    "IceUtil/unicode",
    "IceUtil/inputUtil", 
    "IceUtil/uuid",
    "IceUtil/timer",
    "Slice/errorDetection",
    "Slice/keyword",
    "Ice/proxy",
    "Ice/operations",
    "Ice/exceptions",
    "Ice/inheritance",
    "Ice/facets",
    "Ice/objects",
    "Ice/binding",
    "Ice/faultTolerance",
    "Ice/location",
    "Ice/adapterDeactivation",
    "Ice/slicing/exceptions",
    "Ice/slicing/objects",
    "Ice/gc",
    "Ice/checksum",
    "Ice/stream",
    "Ice/hold",
    "Ice/custom",
    "Ice/retry",
    "Ice/timeout",
    "Ice/servantLocator",
    "Ice/threads",
    "Ice/interceptor",
    "Ice/stringConverter",
    "IceSSL/configuration",
    "Freeze/dbmap",
    "Freeze/complex",
    "Freeze/evictor",
    "Freeze/oldevictor",
    "IceStorm/single",
    "IceStorm/federation",
    "IceStorm/federation2",
    "IceStorm/stress",
    "FreezeScript/dbmap",
    "FreezeScript/evictor",
    "IceGrid/simple",
    #"IceGrid/deployer",
    "IceGrid/session",
    "IceGrid/update",
    "IceGrid/activation",
    "IceGrid/replicaGroup",
    "IceGrid/replication",
    "IceGrid/allocation",
    "IceGrid/distribution",
    "Glacier2/router",
    "Glacier2/attack",
    "Glacier2/sessionControl",
    "Glacier2/ssl",
    "Glacier2/dynamicFiltering",
    "Glacier2/staticFiltering",
    ]

#
# These tests are currently disabled on cygwin
#
if isCygwin() == 0:
    tests += [
      ]

if isWin32():
    tests.insert(0, "IceUtil/condvar")

def usage():
    print "usage: %s " % (sys.argv[0])
    print "  --all                   Run all permutations of the tests."
    print "  --start=<regex>         Start running the tests at the given test."
    print "  --start-after=<regex>   Start running the tests after the given test."
    print "  --loop                  Run the tests in a loop."
    print "  --filter=<regex>        Run all the tests that match the given regex."
    print "  --rfilter=<regex>       Run all the tests that do not match the given regex."
    print "  --debug                 Display debugging information on each test."
    print "  --protocol=tcp|ssl      Run with the given protocol."
    print "  --compress              Run the tests with protocol compression."
    print "  --host=host             Set --Ice.Default.Host=<host>."
    print "  --threadPerConnection   Run with thread-per-connection concurrency model."
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "lr:R:",
        ["start=", "start-after=", "filter=", "rfilter=", "all", "loop", "debug", "protocol=",
         "compress", "host=", "threadPerConnection"])
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
loop = False
arg = ""
all = False
for o, a in opts:
    if o in ("-l", "--loop"):
        loop = True
    elif o in ("-r", "-R", "--filter", '--rfilter'):
        regexp = re.compile(a)
        if o in ("--rfilter", "-R"):
            tests = [ x for x in tests if not regexp.search(x) ]
        else:
            tests = [ x for x in tests if regexp.search(x) ]
    elif o == "--all" :
        all = True
    elif o in ( "--protocol", "--host", "--debug", "--compress", "--threadPerConnection" ):
        if o == "--protocol":
            if a not in ( "ssl", "tcp"):
                usage()
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

args = []
if all and len(arg) > 0:
    usage()

if all:
    for protocol in ["ssl", "tcp"]:
        for compress in [0, 1]:
            for threadPerConnection in [0, 1]:
                arg = ""
                if compress:
                    arg += "--compress"
                if threadPerConnection:
                    arg += " --threadPerConnection"
                arg += " --protocol %s" % (protocol)
                args.append(arg)
else:
    args.append(arg)

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
