#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice

Ice.loadSlice('Test.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, ref):
    manager = Test.ServerManagerPrx.checkedCast(communicator.stringToProxy(ref))
    test(manager)

    print "testing stringToProxy... ",
    base = communicator.stringToProxy("test @ TestAdapter")
    base2 = communicator.stringToProxy("test @ TestAdapter")
    base3 = communicator.stringToProxy("test")
    base4 = communicator.stringToProxy("ServerManager")
    print "ok"

    print "starting server... ",
    manager.startServer()
    print "ok"

    print "testing checked cast... ",
    obj = Test.TestIntfPrx.checkedCast(base)
    obj = Test.TestIntfPrx.checkedCast(communicator.stringToProxy("test@TestAdapter"))
    obj = Test.TestIntfPrx.checkedCast(communicator.stringToProxy("test   @TestAdapter"))
    obj = Test.TestIntfPrx.checkedCast(communicator.stringToProxy("test@   TestAdapter"))
    test(obj)
    obj2 = Test.TestIntfPrx.checkedCast(base2)
    test(obj2)
    obj3 = Test.TestIntfPrx.checkedCast(base3)
    test(obj3)
    obj4 = Test.ServerManagerPrx.checkedCast(base4)
    test(obj4)
    print "ok"

    print "testing object reference from server... ",
    hello = obj.getHello()
    hello.sayHello()
    print "ok"

    print "shutdown server... ",
    obj.shutdown()
    print "ok"

    print "restarting server... ",
    manager.startServer()
    print "ok"

    print "testing whether server is still reachable... ",
    try:
	obj2 = Test.TestIntfPrx.checkedCast(base2)
	obj2.ice_ping()
    except Ice.LocalException:
	test(False)
    print "ok"    

    print "testing object reference from server... ",
    hello.sayHello()
    print "ok"

    print "testing reference with unknown identity... ",
    try:
	base = communicator.stringToProxy("unknown/unknown")
	base.ice_ping()
	test(False)
    except Ice.NotRegisteredException, ex:
	test(ex.kindOfObject == "object")
	test(ex.id == "unknown/unknown")
    print "ok"

    print "testing reference with unknown adapter... ",
    try:
	base = communicator.stringToProxy("test @ TestAdapterUnknown")
	base.ice_ping()
	test(False)
    except Ice.NotRegisteredException, ex:
	test(ex.kindOfObject == "object adapter")
	test(ex.id == "TestAdapterUnknown")
    print "ok"

    print "shutdown server... ",
    obj.shutdown()
    print "ok"

    print "testing whether server is gone... ",
    try:
	obj2.ice_ping()
	test(False)
    except Ice.LocalException:
	print "ok"

    print "shutdown server manager... ",
    manager.shutdown()
    print "ok"
