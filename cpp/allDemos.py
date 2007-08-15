#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def runDemos(args, demos, num = 0):
    rootPath = "demo"
    if not os.path.exists(rootPath):
        rootPath = "."
        
    if not os.path.exists(os.path.join(rootPath, os.path.normpath(demos[0]))):
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

	status = os.system("cd %s ; %s %s" % (dir, "./expect.py", args))

        if status:
            if(num > 0):
                print "[" + str(num) + "]",
            print "test in " + dir + " failed with exit status", status,
            sys.exit(status)

#
# List of all basic demos.
#
demos = [ "IceUtil/workqueue",
          "Ice/async",
          "Ice/bidir",
          "Ice/callback",
          "Ice/converter",
          "Ice/hello",
          "Ice/invoke",
          "Ice/latency",
          "Ice/minimal",
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
    print "usage: " + sys.argv[0] + " --fast --trace --start=<demo> -l -r <regex> -R <regex> --debug --host host"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "lr:R:", ["start=", "fast", "trace", "debug", "host="])
except getopt.GetoptError:
    usage()

if(args):
    usage()

loop = 0
args = ""
for o, a in opts:
    if o == "-l":
        loop = 1
    if o == "-r" or o == '-R':
        import re
        regexp = re.compile(a)
        if o == '-r':
            def rematch(x): return regexp.search(x)
        else:
            def rematch(x): return not regexp.search(x)
        demos = filter(rematch, demos)
    if o == "--protocol":
        if a not in ( "ssl", "tcp"):
            usage()
        args += " " + o + " " + a
    if o == "--host" :
        args += " " + o + " " + a
    if o in ( "--fast", "--trace", "--debug"):
        args += " " + o 
    if o == '--start':
        import re
        regexp = re.compile(a)
        found = False
        nt = []
        for t in demos:
            if not found and regexp.search(t):
                found = True
            if found:
                nt.append(t)
        if len(nt) == 0:
            print "test %s not found. no demos to run" % (a)
            sys.exit(2)
        demos = nt
        
if loop:
    num = 1
    while 1:
        runDemos(args, demos, num)
        num += 1
else:
    runDemos(args, demos)
