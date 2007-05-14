#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys
import getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

def runTests(args, tests, num = 0):

    #
    # Run each of the tests.
    #
    for i in tests:

        i = os.path.normpath(i)
        dir = os.path.join(toplevel, "test", i)

        print
        if(num > 0):
            print "[" + str(num) + "]",
        print "*** running tests in " + dir,
        print

        status = os.system(os.path.join(dir, "run.py " + args))

        if status and not (sys.platform.startswith("aix") and status == 256):
            if(num > 0):
                print "[" + str(num) + "]",
            print "test in " + dir + " failed with exit status", status,
            sys.exit(status)

#
# List of all basic tests.
#
tests = [ \
    "Ice/adapterDeactivation", \
    "Ice/binding", \
    "Ice/exceptions", \
    "Ice/facets", \
    "Ice/faultTolerance", \
    "Ice/inheritance", \
    "Ice/location", \
    "Ice/objects", \
    "Ice/proxy", \
    "Ice/operations", \
    "Ice/slicing/exceptions", \
    "Ice/slicing/objects", \
    "Ice/custom", \
    "Ice/checksum", \
    "Ice/retry", \
    "Ice/timeout", \
    "Ice/servantLocator", \
    ]

def usage():
    print "usage: " + sys.argv[0] + " -l -r <regex> -R <regex> --debug --protocol protocol --compress --host host --threadPerConnection"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "lr:R:", \
        ["debug", "protocol=", "compress", "host=", "threadPerConnection"])
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
        tests = filter(rematch, tests)
    if o == "--protocol":
        if a not in ( "ssl", "tcp"):
            usage()
        args += " " + o + " " + a
    if o == "--host" :
        args += " " + o + " " + a
    if o in ( "--debug", "--compress", "--threadPerConnection" ):
        args += " " + o 
    

if loop:
    num = 1
    while 1:
        runTests(args, tests, num)
        num += 1
else:
    runTests(args, tests)
