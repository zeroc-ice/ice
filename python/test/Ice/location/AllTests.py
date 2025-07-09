# Copyright (c) ZeroC, Inc.

import sys
import uuid

import Test

import Ice


class HelloI(Test.Hello):
    def sayHello(self, current):
        pass


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def allTests(helper, communicator):
    manager = Test.ServerManagerPrx(communicator, f"ServerManager:{helper.getTestEndpoint()}")
    locator = communicator.getDefaultLocator()
    test(manager)

    registry = Test.TestLocatorRegistryPrx.checkedCast(locator.getRegistry())
    test(registry)

    sys.stdout.write("testing proxy creation... ")
    sys.stdout.flush()
    base = Ice.ObjectPrx(communicator, "test @ TestAdapter")
    base2 = Ice.ObjectPrx(communicator, "test @ TestAdapter")
    base3 = Ice.ObjectPrx(communicator, "test")
    base4 = Ice.ObjectPrx(communicator, "ServerManager")
    base5 = Ice.ObjectPrx(communicator, "test2")
    print("ok")

    sys.stdout.write("testing ice_locator and ice_getLocator...  ")
    sys.stdout.flush()
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), communicator.getDefaultLocator()))
    anotherLocator = Ice.LocatorPrx(communicator, "anotherLocator")
    base = base.ice_locator(anotherLocator)
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), anotherLocator))
    communicator.setDefaultLocator(None)
    base = Ice.ObjectPrx(communicator, "test @ TestAdapter")
    test(not base.ice_getLocator())
    base = base.ice_locator(anotherLocator)
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), anotherLocator))
    communicator.setDefaultLocator(locator)
    base = Ice.ObjectPrx(communicator, "test @ TestAdapter")
    test(Ice.proxyIdentityEqual(base.ice_getLocator(), communicator.getDefaultLocator()))

    #
    # We also test ice_router/ice_getRouter (perhaps we should add a
    # test/Ice/router test?)
    #
    test(not base.ice_getRouter())
    anotherRouter = Ice.RouterPrx(communicator, "anotherRouter")
    base = base.ice_router(anotherRouter)
    test(Ice.proxyIdentityEqual(base.ice_getRouter(), anotherRouter))
    router = Ice.RouterPrx(communicator, "dummyrouter")
    communicator.setDefaultRouter(router)
    base = Ice.ObjectPrx(communicator, "test @ TestAdapter")
    test(Ice.proxyIdentityEqual(base.ice_getRouter(), communicator.getDefaultRouter()))
    communicator.setDefaultRouter(None)
    base = Ice.ObjectPrx(communicator, "test @ TestAdapter")
    test(not base.ice_getRouter())
    print("ok")

    sys.stdout.write("starting server... ")
    sys.stdout.flush()
    manager.startServer()
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    obj = Test.TestIntfPrx.checkedCast(base)
    obj = Test.TestIntfPrx.checkedCast(Ice.ObjectPrx(communicator, "test@TestAdapter"))
    obj = Test.TestIntfPrx.checkedCast(Ice.ObjectPrx(communicator, "test   @TestAdapter"))
    obj = Test.TestIntfPrx.checkedCast(Ice.ObjectPrx(communicator, "test@   TestAdapter"))
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
        base = Ice.ObjectPrx(communicator, "unknown/unknown")
        base.ice_ping()
        test(False)
    except Ice.NotRegisteredException as ex:
        test(ex.kindOfObject == "object")
        test(ex.id == "unknown/unknown")
    print("ok")

    sys.stdout.write("testing reference with unknown adapter... ")
    sys.stdout.flush()
    try:
        base = Ice.ObjectPrx(communicator, "test @ TestAdapterUnknown")
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
    hello = Test.HelloPrx(communicator, "hello")
    obj.migrateHello()
    hello.ice_getConnection().close().result()
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

    sys.stdout.write("testing indirect references to collocated objects... ")
    sys.stdout.flush()
    communicator.getProperties().setProperty("Hello.AdapterId", str(uuid.uuid4()))
    adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default")

    id = Ice.Identity()
    id.name = str(uuid.uuid4())
    adapter.add(HelloI(), id)

    helloPrx = Test.HelloPrx(communicator, Ice.identityToString(id))
    test(not helloPrx.ice_getConnection())

    helloPrx = Test.HelloPrx.checkedCast(adapter.createIndirectProxy(id))
    test(not helloPrx.ice_getConnection())

    helloPrx = Test.HelloPrx.checkedCast(adapter.createDirectProxy(id))
    test(not helloPrx.ice_getConnection())

    print("ok")

    sys.stdout.write("shutdown server manager... ")
    sys.stdout.flush()
    manager.shutdown()
    print("ok")
