#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../..", "../../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine()

name = os.path.join("Ice", "exceptions")
testdir = os.path.dirname(os.path.abspath(__file__))
nameAMD = os.path.join("Ice", "exceptionsAMD")
testdirAMD = testdir + "AMD"

print "tests with regular server."
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + os.pathsep + classpath
TestUtil.clientServerTest(name)

print "tests with AMD server."
TestUtil.clientServerTestWithClasspath(name,\
    os.path.join(testdirAMD, "classes") + os.pathsep + classpath, \
    os.path.join(testdir, "classes") + os.pathsep + classpath)

print "tests with collocated server."
TestUtil.collocatedTest(name)
sys.exit(0)
