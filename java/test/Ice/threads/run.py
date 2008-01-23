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

if TestUtil.getIceSSLVersion() == "1.4":
    print "Detected IceSSL version that requires thread-per-connection, skipping test."
    sys.exit(0)

name = os.path.join("Ice", "threads")
testdir = os.path.dirname(os.path.abspath(__file__))

classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + os.pathsep + classpath
TestUtil.clientServerTest(name)

sys.exit(0)
