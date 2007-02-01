#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, ref):
    manager = Test.ServerManagerPrx.checkedCast(communicator.stringToProxy(ref))
    locator = communicator.getDefaultLocator()
    test(manager)

    print "testing stringToProxy...",
    base = communicator.stringToProxy("test @ TestAdapter")
    base2 = communicator.stringToProxy("test @ TestAdapter")
    base3 = communicator.stringToProxy("test")
    base4 = communicator.stringToProxy("ServerManager")
    base5 = communicator.stringToProxy("test2")
    print "ok"

    print "testing ice_locator and ice_getLocator... ",
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), communicator.getDefaultLocator()));
    anotherLocator = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy("anotherLocator"));
    base = base.ice_locator(anotherLocator);
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), anotherLocator));
    communicator.setDefaultLocator(None);
    base = communicator.stringToProxy("test @ TestAdapter");
    test(not base.ice_getLocator());
    base = base.ice_locator(anotherLocator);
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), anotherLocator));
    communicator.setDefaultLocator(locator);
    base = communicator.stringToProxy("test @ TestAdapter");
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), communicator.getDefaultLocator())); 
        
    #
    # We also test ice_router/ice_getRouter (perhaps we should add a
    # test/Ice/router test?)
    #
    test(not base.ice_getRouter());
    anotherRouter = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("anotherRouter"));
    base = base.ice_router(anotherRouter);
    test(Ice.proxyIdentityEqual(base.ice_getRouter(), anotherRouter));
    router = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("dummyrouter"));
    communicator.setDefaultRouter(router);
    base = communicator.stringToProxy("test @ TestAdapter");
    test(Ice.proxyIdentityEqual(base.ice_getRouter(), communicator.getDefaultRouter()));
    communicator.setDefaultRouter(None);
    base = communicator.stringToProxy("test @ TestAdapter");
    test(not base.ice_getRouter());
    print "ok"

    print "starting server...",
    manager.startServer()
    print "ok"

    print "testing checked cast...",
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
    obj5 = Test.TestIntfPrx.checkedCast(base5)
    test(obj5)
    print "ok"

    print "testing id@AdapterId indirect proxy...",
    obj.shutdown()
    manager.startServer()
    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException:
        test(False)
    print "ok"

    print "testing identity indirect proxy...",
    obj.shutdown()
    manager.startServer()
    try:
        obj3 = Test.TestIntfPrx.checkedCast(base3)
        obj3.ice_ping()
    except Ice.LocalException:
        test(False)
    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException:
        test(False)
    obj.shutdown()
    manager.startServer()
    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException:
        test(False)
    try:
        obj3 = Test.TestIntfPrx.checkedCast(base3)
        obj3.ice_ping()
    except Ice.LocalException:
        test(False)
    obj.shutdown()
    manager.startServer()

    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException:
        test(False)
    obj.shutdown()
    manager.startServer()
    try:
        obj3 = Test.TestIntfPrx.checkedCast(base3)
        obj3.ice_ping()
    except Ice.LocalException:
        test(False)
    obj.shutdown()
    manager.startServer()
    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException:
        test(False)
    obj.shutdown()
    manager.startServer()

    try:
        obj5 = Test.TestIntfPrx.checkedCast(base5)
        obj5.ice_ping()
    except Ice.LocalException:
        test(False)
    print "ok"

    print "testing reference with unknown identity...",
    try:
        base = communicator.stringToProxy("unknown/unknown")
        base.ice_ping()
        test(False)
    except Ice.NotRegisteredException, ex:
        test(ex.kindOfObject == "object")
        test(ex.id == "unknown/unknown")
    print "ok"

    print "testing reference with unknown adapter...",
    try:
        base = communicator.stringToProxy("test @ TestAdapterUnknown")
        base.ice_ping()
        test(False)
    except Ice.NotRegisteredException, ex:
        test(ex.kindOfObject == "object adapter")
        test(ex.id == "TestAdapterUnknown")
    print "ok"

    print "testing object reference from server...",
    hello = obj.getHello()
    hello.sayHello()
    print "ok"

    print "testing object reference from server after shutdown...",
    obj.shutdown()
    manager.startServer()
    hello.sayHello()
    print "ok"

    print "testing object migration...",
    hello = Test.HelloPrx.checkedCast(communicator.stringToProxy("hello"))
    obj.migrateHello()
    hello.sayHello()
    obj.migrateHello()
    hello.sayHello()
    obj.migrateHello()
    hello.sayHello()
    print "ok"

    print "shutdown server...",
    obj.shutdown()
    print "ok"

    print "testing whether server is gone...",
    try:
        obj2.ice_ping()
        test(False)
    except Ice.LocalException:
        pass
    try:
        obj3.ice_ping()
        test(False)
    except Ice.LocalException:
        pass
    try:
        obj5.ice_ping()
        test(False)
    except Ice.LocalException:
        pass
    print "ok"

    #
    # Collocated invocations are not supported in Python.
    #
    #print "testing indirect references to collocated objects...",

    print "shutdown server manager...",
    manager.shutdown()
    print "ok"
