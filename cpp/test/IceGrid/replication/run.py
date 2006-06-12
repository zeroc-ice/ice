#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
import IceGridAdmin

name = os.path.join("IceGrid", "replication")
testdir = os.path.join(toplevel, "test", name)

if TestUtil.isWin32():
    os.environ["PATH"] = testdir + ";" + os.getenv("PATH", "")
elif TestUtil.isAIX():
    os.environ["LIBPATH"] = testdir + ":" + os.getenv("LIBPATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = testdir + ":" + os.getenv("LD_LIBRARY_PATH", "")
    os.environ["LD_LIBRARY_PATH_64"] = testdir + ":" + os.getenv("LD_LIBRARY_PATH_64", "")

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
# NOTE: We also set the retry intervals to retry several times. This
# is necessary for the test to work. The tests invoke successively on
# the same proxy and shutdown the server on each invocation. It takes
# a bit of time for the IceGrid node to detect that the server is down
# and to return a direct proxy from an active server.
#
additionalOptions = " --Ice.Default.Locator=\"IceGrid/Locator:default -p 12010\"" + \
                    " --Ice.PrintAdapterReady=0 --Ice.PrintProcessId=0 --Ice.RetryIntervals=\"0 50 100 250\""

IceGridAdmin.iceGridTest(name, "application.xml", "--Ice.RetryIntervals=\"0 50 100 250\"", \
                         "icebox.exe=" + TestUtil.getIceBox(testdir))
sys.exit(0)
