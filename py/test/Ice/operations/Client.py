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
    myClass = AllTests.allTests(communicator)

    print "testing server shutdown... ",
    myClass.shutdown()
    try:
        myClass.opVoid()
        test(False)
    except Ice.LocalException:
        print "ok"

    return True

try:
    #
    # In this test, we need at least two threads in the
    # client side thread pool for nested AMI.
    #
    properties = Ice.getDefaultProperties(sys.argv)
    properties.setProperty('Ice.ThreadPool.Client.Size', '2')
    properties.setProperty('Ice.ThreadPool.Client.SizeWarn', '0')

    communicator = Ice.initialize(sys.argv)
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
