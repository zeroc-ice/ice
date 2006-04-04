#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

def usage():
    print "usage: " + sys.argv[0] + " [-m]"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "m")
except getopt.GetoptError:
    usage()

mono = 0
for o, a in opts:
    if o == "-m":
        mono = 1

if not TestUtil.isWin32():
    mono = 1

name = os.path.join("Ice", "timeout")

TestUtil.clientServerTest(mono, name)
sys.exit(0)
