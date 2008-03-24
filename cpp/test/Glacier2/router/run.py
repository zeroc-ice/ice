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

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine()

router = os.path.join(TestUtil.getCppBinDir(), "glacier2router")

def startRouter(buffered):

    args = r' --Ice.Warn.Dispatch=0' + \
           r' --Ice.Warn.Connections=0' + \
           r' --Glacier2.Filter.Category.Accept="c1 c2"' + \
           r' --Glacier2.Filter.Category.AcceptUser="2"' + \
           r' --Glacier2.SessionTimeout="30"' + \
           r' --Glacier2.Client.Endpoints="default -p 12347 -t 10000"' + \
           r' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1 -t 10000"' \
           r' --Ice.Admin.Endpoints="tcp -h 127.0.0.1 -p 12348 -t 10000"' + \
           r' --Ice.Admin.InstanceName="Glacier2"' + \
           r' --Glacier2.CryptPasswords="' + TestUtil.getMappingDir(__file__) + r'/test/Glacier2/router/passwords"'

    if buffered:
        args += ' --Glacier2.Client.Buffered=1 --Glacier2.Server.Buffered=1' 
        print "starting router in buffered mode...",
    else:
        args += ' --Glacier2.Client.Buffered=0 --Glacier2.Server.Buffered=0'
        print "starting router in unbuffered mode...",

    starterPipe = TestUtil.startServer(router, args + " 2>&1")
    TestUtil.getServerPid(starterPipe)

    #
    # For this test we don't want to add the router to the server threads
    # since we want the the router to run over two calls to
    # mixedClientServerTest
    #
    TestUtil.getAdapterReady(starterPipe, False, 2)
    print "ok"

    routerThread = TestUtil.ReaderThread(starterPipe);
    routerThread.start()
    
    return routerThread

name = os.path.join("Glacier2", "router")

#
# We first run the test with unbuffered mode.
#
routerThread = startRouter(False)
TestUtil.mixedClientServerTestWithOptions(name, "", " --shutdown")
routerThread.join()
if routerThread.getStatus():
    sys.exit(1)

#
# Then we run the test in buffered mode.
# 
routerThread = startRouter(True)
TestUtil.mixedClientServerTest(name)

#
# We run the test again, to check whether the glacier router can
# handle multiple clients. Also, when we run for the second time, we
# want the client to shutdown the router after running the tests.
#
TestUtil.mixedClientServerTestWithOptions(name, "", " --shutdown")

routerThread.join()
if routerThread.getStatus():
    sys.exit(1)

sys.exit(0)
