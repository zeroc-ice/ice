# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, AllTests

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def run(args, communicator):
    AllTests.allTests(communicator, "ServerManager:default -p 12345 -t 10000")
    return True

try:
    properties = Ice.createProperties(sys.argv)
    properties.setProperty("Ice.Default.Locator", "locator:default -p 12345")
    communicator = Ice.initializeWithProperties(sys.argv, properties)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
