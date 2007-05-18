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

def isCygwin():

    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    if sys.platform[:6] == "cygwin":
        return 1
    else:
        return 0

def isWin32():
    if sys.platform == "win32" or isCygwin():
        return 1
    else:
        return 0

def isVista():
    if isWin32() and sys.getwindowsversion()[0] == 6:
        return 1
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
        if(num > 0):
            print "[" + str(num) + "]",
        print "*** running tests in " + dir,
        print

        status = os.system("python " + os.path.join(dir, "run.py " + args))

        if status:
            if(num > 0):
                print "[" + str(num) + "]",
            print "test in " + dir + " failed with exit status", status,
            sys.exit(status)

#
# List of all basic tests.
#
tests = [ \
    "IceUtil/inputUtil", \
    "Ice/proxy", \
    "Ice/operations", \
    "Ice/exceptions", \
    "Ice/inheritance", \
    "Ice/facets", \
    "Ice/hold", \
    "Ice/objects", \
    "Ice/binding", \
    "Ice/faultTolerance", \
    "Ice/location", \
    "Ice/adapterDeactivation", \
    "Ice/slicing/exceptions", \
    "Ice/slicing/objects", \
    "Ice/checksum", \
    "Ice/stream", \
    "Ice/retry", \
    "Ice/timeout", \
    "Ice/servantLocator", \
    "IceSSL/configuration", \
    "Ice/threads", \
    "Glacier2/router", \
    "Glacier2/attack", \
    "IceGrid/simple" \
    ]

def usage():
    print "usage: " + sys.argv[0] + " -m|--mono -l -r <regex> -R <regex> --debug --protocol protocol --compress --host host --threadPerConnection"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "lmr:R:", \
        ["mono", "debug", "protocol=", "compress", "host=", "threadPerConnection"])
except getopt.GetoptError:
    usage()

if(args):
    usage()

mono = 0
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
    if o in ( "-m", "--mono" ):
        args += " " + o
        mono = 1
    if o in ( "--debug", "-m", "--mono", "--compress", "--threadPerConnection" ):
        args += " " + o

if not isWin32():
    mono = 1

# For mono or vista the IceSSL configuration test is removed.
if mono or isVista():
    try:
	tests.remove("IceSSL/configuration")
    except ValueError:
	pass

if loop:
    num = 1
    while 1:
        runTests(args, tests, num)
        num += 1
else:
    runTests(args, tests)
