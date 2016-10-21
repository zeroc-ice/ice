# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys

class HelloI(Test.Hello):
    def sayHello(self, current=None):
        pass

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, ref):
    manager = Test.ServerManagerPrx.checkedCast(communicator.stringToProxy(ref))
    locator = communicator.getDefaultLocator()
    test(manager)

    registry = Test.TestLocatorRegistryPrx.checkedCast(locator.getRegistry());
    test(registry);

    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    base = communicator.stringToProxy("test @ TestAdapter")
    base2 = communicator.stringToProxy("test @ TestAdapter")
    base3 = communicator.stringToProxy("test")
    base4 = communicator.stringToProxy("ServerManager")
    base5 = communicator.stringToProxy("test2")
    print("ok")

    sys.stdout.write("testing ice_locator and ice_getLocator...  ")
    sys.stdout.flush()
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
    print("ok")

    sys.stdout.write("starting server... ")
    sys.stdout.flush()
    manager.startServer()
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
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
    print("ok")

    sys.stdout.write("testing id@AdapterId indirect proxy... ")
    sys.stdout.flush()
    obj.shutdown()
    manager.startServer()
    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException:
        test(False)
    print("ok")

    sys.stdout.write("testing identity indirect proxy... ")
    sys.stdout.flush()
    obj.shutdown()
    manager.startServer()
    try:
        obj3 = Test.TestIntfPrx.checkedCast(base3)
        obj3.ice_ping()
    except Ice.LocalException as ex:
        print(ex)
        test(False)
    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException as ex:
        print(ex)
        test(False)
    obj.shutdown()
    manager.startServer()
    try:
        obj2 = Test.TestIntfPrx.checkedCast(base2)
        obj2.ice_ping()
    except Ice.LocalException as ex:
        print(ex)
        test(False)
    try:
        obj3 = Test.TestIntfPrx.checkedCast(base3)
        obj3.ice_ping()
    except Ice.LocalException as ex:
        print(ex)
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
    print("ok")

    sys.stdout.write("testing reference with unknown identity... ")
    sys.stdout.flush()
    try:
        base = communicator.stringToProxy("unknown/unknown")
        base.ice_ping()
        test(False)
    except Ice.NotRegisteredException as ex:
        test(ex.kindOfObject == "object")
        test(ex.id == "unknown/unknown")
    print("ok")

    sys.stdout.write("testing reference with unknown adapter... ")
    sys.stdout.flush()
    try:
        base = communicator.stringToProxy("test @ TestAdapterUnknown")
        base.ice_ping()
        test(False)
    except Ice.NotRegisteredException as ex:
        test(ex.kindOfObject == "object adapter")
        test(ex.id == "TestAdapterUnknown")
    print("ok")

    sys.stdout.write("testing object reference from server... ")
    sys.stdout.flush()
    hello = obj.getHello()
    hello.sayHello()
    print("ok")

    sys.stdout.write("testing object reference from server after shutdown... ")
    sys.stdout.flush()
    obj.shutdown()
    manager.startServer()
    hello.sayHello()
    print("ok")

    sys.stdout.write("testing object migration... ")
    sys.stdout.flush()
    hello = Test.HelloPrx.checkedCast(communicator.stringToProxy("hello"))
    obj.migrateHello()
    hello.ice_getConnection().close(False);
    hello.sayHello()
    obj.migrateHello()
    hello.sayHello()
    obj.migrateHello()
    hello.sayHello()
    print("ok")

    sys.stdout.write("shutdown server... ")
    sys.stdout.flush()
    obj.shutdown()
    print("ok")

    sys.stdout.write("testing whether server is gone... ")
    sys.stdout.flush()
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
    print("ok")

    #
    # Set up test for calling a collocated object through an indirect, adapterless reference.
    #
    sys.stdout.write("testing indirect references to collocated objects... ")
    sys.stdout.flush()
    properties = communicator.getProperties();
    properties.setProperty("Ice.PrintAdapterReady", "0");
    adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default");
    adapter.setLocator(locator);
    assert(adapter.getLocator() == locator)

    id = Ice.Identity();
    id.name = Ice.generateUUID();
    registry.addObject(adapter.add(HelloI(), id));
    adapter.activate();

    helloPrx = Test.HelloPrx.checkedCast(communicator.stringToProxy(communicator.identityToString(id)));
    test(not helloPrx.ice_getConnection());

    adapter.deactivate();
    print("ok")

    sys.stdout.write("shutdown server manager... ")
    sys.stdout.flush()
    manager.shutdown()
    print("ok")
