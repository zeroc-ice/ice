#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
Ice.loadSlice('Test.ice')
import Test
import RouterI

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def run(args, communicator, sync):
    hello = Test.HelloPrx.checkedCast(communicator.stringToProxy("test:default -p 12010"))
    hello.sayHello(False)
    hello.sayHello(False, { "_fwd":"o" } )
    test(hello.add(10, 20) == 30)
    try:
        hello.raiseUE()
        test(False)
    except Test.UE:
        pass

    try:
        Test.HelloPrx.checkedCast(communicator.stringToProxy("unknown:default -p 12010 -t 10000"))
        test(False)
    except Ice.ObjectNotExistException:
        pass

    # First try an object at a non-existent endpoint.
    try:
        Test.HelloPrx.checkedCast(communicator.stringToProxy("missing:default -p 12000 -t 10000"))
        test(False)
    except Ice.UnknownLocalException as e:
        test(e.unknown.find('ConnectionRefusedException'))
    if sync:
        hello.shutdown()
    return True

argv = sys.argv[:]  # Clone the arguments to use again later

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(argv)
    initData.properties.setProperty('Ice.Warn.Dispatch', '0')
    communicator = Ice.initialize(argv, initData)
    router = RouterI.RouterI(communicator, False)
    sys.stdout.write("testing async blobject... ")
    sys.stdout.flush()
    status = run(sys.argv, communicator, False)
    print("ok")
    router.destroy()
except:
    traceback.print_exc()
    status = False

if communicator:
    communicator.destroy()

if status:
    try:
        initData = Ice.InitializationData()
        initData.properties = Ice.createProperties(sys.argv)
        initData.properties.setProperty('Ice.Warn.Dispatch', '0')
        communicator = Ice.initialize(sys.argv, initData)
        router = RouterI.RouterI(communicator, True)
        sys.stdout.write("testing sync blobject... ")
        sys.stdout.flush()
        status = run(sys.argv, communicator, True)
        print("ok")
        router.destroy()
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
