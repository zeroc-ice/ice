#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt, re

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

if sys.platform == "win32":
    print "allDemos.py only supports cygwin python under Windows (use /usr/bin/python allDemos.py)"
    sys.exit(1)

def runDemos(args, demos, num = 0):

    rootPath = "demo"
    if not os.path.exists(rootPath):
        rootPath = "."
        
    if len(demos) > 0 and not os.path.exists(os.path.join(rootPath, os.path.normpath(demos[0]))):
        print "Unable to locate first demo. Check directory structure and location of scripts"
        sys.exit(1)

    #
    # Run each of the demos.
    #
    for i in demos:

        i = os.path.normpath(i)
        dir = os.path.join(rootPath, i)

        print
        if(num > 0):
            print "[" + str(num) + "]",
        print "*** running demo in " + dir,
        print

        if isCygwin():
            status = os.system("cd %s ; %s %s" % (dir, "/usr/bin/python expect.py", args))
        else:
            status = os.system("cd %s ; %s %s" % (dir, "./expect.py", args))

        if status:
            if(num > 0):
                print "[" + str(num) + "]",
            print "test in " + dir + " failed with exit status", status,
            sys.exit(status)

#
# List of all basic demos.
#
demos = [
    "IceUtil/workqueue",
    "Ice/async",
    "Ice/bidir",
    "Ice/callback",
    "Ice/converter",
    "Ice/hello",
    "Ice/invoke",
    "Ice/latency",
    "Ice/minimal",
    "Ice/multicast",
    "Ice/nested",
    "Ice/session",
    "Ice/throughput",
    "Ice/value",
    "IceBox/hello",
    "IceStorm/clock",
    "IceStorm/counter",
    "IceStorm/replicated",
    "IceGrid/simple",
    "IceGrid/allocate",
    "IceGrid/sessionActivation",
    "IceGrid/replication",
    "Glacier2/chat",
    "Glacier2/callback",
    "Freeze/bench",
    "Freeze/customEvictor",
    "Freeze/phonebook",
    "Freeze/library",
    "Freeze/backup",
    "book/freeze_filesystem",
    "book/simple_filesystem",
    "book/printer",
    "book/lifecycle",
]

#
# These demos are currently disabled on cygwin
#
if isCygwin() == 0:
    demos += [ ]

def usage():
    print "usage: %s " % (sys.argv[0])
    print "  --start=<regex>         Start running the demos at the given demo."
    print "  --start-after=<regex>   Start running the demos after the given demo."
    print "  --loop                  Run the demos in a loop."
    print "  --filter=<regex>        Run all the demos that match the given regex."
    print "  --rfilter=<regex>       Run all the demos that do not match the given regex."
    print "  --fast                  Run an abbreviated version of the demos."
    print "  --debug                 Display debugging information on each demos."
    print "  --trace                 Run the demos with tracing enabled."
    print "  --host=host             Set --Ice.Default.Host=<host>."
    print "  --mode=debug|release    Run the demos with debug or release mode builds (win32 only)."
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "lr:R:", [
            "filter=", "rfilter=", "start-after=", "start=", "loop", "fast", "trace", "debug", "host=", "mode="])
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

loop = False
arg = ""
for o, a in opts:
    if o in ("-l", "--loop"):
        loop = True
    elif o in ("-r", "-R", "--filter", '--rfilter'):
        regexp = re.compile(a)
        if o in ("--rfilter", "-R"):
            demos = [ x for x in demos if not regexp.search(x) ]
        else:
            demos = [ x for x in demos if regexp.search(x) ]
    elif o in ("--host", "--fast", "--trace", "--debug", "--mode"):
        if o == "--mode":
            if a not in ( "debug", "release"):
                usage()
        arg += " " + o
        if len(a) > 0:
            arg += " " + a
    elif o in ('--start', "--start-after"):
        start = index(demos, re.compile(a))
        if start == -1:
            print "demo %s not found. no demos to run" % (a)
            sys.exit(2)
        if o == "--start-after":
            start += 1
        demos = demos[start:]
        
if loop:
    num = 1
    while 1:
        runDemos(arg, demos, num)
        num += 1
else:
    runDemos(arg, demos)
