# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def allTests(communicator, ref)
    manager = Test::ServerManagerPrx::checkedCast(communicator.stringToProxy(ref))
    locator = communicator.getDefaultLocator()
    test(manager)

    print "testing stringToProxy... "
    STDOUT.flush
    base = communicator.stringToProxy("test @ TestAdapter")
    base2 = communicator.stringToProxy("test @ TestAdapter")
    base3 = communicator.stringToProxy("test")
    base4 = communicator.stringToProxy("ServerManager")
    base5 = communicator.stringToProxy("test2")
    puts "ok"

    print "testing ice_locator and ice_getLocator... "
    STDOUT.flush
    test(Ice::proxyIdentityEqual(base.ice_getLocator(), communicator.getDefaultLocator()))
    anotherLocator = Ice::LocatorPrx::uncheckedCast(communicator.stringToProxy("anotherLocator"))
    base = base.ice_locator(anotherLocator)
    test(Ice::proxyIdentityEqual(base.ice_getLocator(), anotherLocator))
    communicator.setDefaultLocator(nil)
    base = communicator.stringToProxy("test @ TestAdapter")
    test(!base.ice_getLocator())
    base = base.ice_locator(anotherLocator)
    test(Ice::proxyIdentityEqual(base.ice_getLocator(), anotherLocator))
    communicator.setDefaultLocator(locator)
    base = communicator.stringToProxy("test @ TestAdapter")
    test(Ice::proxyIdentityEqual(base.ice_getLocator(), communicator.getDefaultLocator()));

    #
    # We also test ice_router/ice_getRouter (perhaps we should add a
    # test/Ice/router test?)
    #
    test(!base.ice_getRouter())
    anotherRouter = Ice::RouterPrx::uncheckedCast(communicator.stringToProxy("anotherRouter"))
    base = base.ice_router(anotherRouter)
    test(Ice::proxyIdentityEqual(base.ice_getRouter(), anotherRouter))
    router = Ice::RouterPrx::uncheckedCast(communicator.stringToProxy("dummyrouter"))
    communicator.setDefaultRouter(router)
    base = communicator.stringToProxy("test @ TestAdapter")
    test(Ice::proxyIdentityEqual(base.ice_getRouter(), communicator.getDefaultRouter()))
    communicator.setDefaultRouter(nil)
    base = communicator.stringToProxy("test @ TestAdapter")
    test(!base.ice_getRouter())
    puts "ok"

    print "starting server... "
    STDOUT.flush
    manager.startServer()
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    obj = Test::TestIntfPrx::checkedCast(base)
    obj = Test::TestIntfPrx::checkedCast(communicator.stringToProxy("test@TestAdapter"))
    obj = Test::TestIntfPrx::checkedCast(communicator.stringToProxy("test   @TestAdapter"))
    obj = Test::TestIntfPrx::checkedCast(communicator.stringToProxy("test@   TestAdapter"))
    test(obj)
    obj2 = Test::TestIntfPrx::checkedCast(base2)
    test(obj2)
    obj3 = Test::TestIntfPrx::checkedCast(base3)
    test(obj3)
    obj4 = Test::ServerManagerPrx::checkedCast(base4)
    test(obj4)
    obj5 = Test::TestIntfPrx::checkedCast(base5)
    test(obj5)
    puts "ok"

    print "testing id@AdapterId indirect proxy... "
    STDOUT.flush
    obj.shutdown()
    manager.startServer()
    begin
        obj2 = Test::TestIntfPrx::checkedCast(base2)
        obj2.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    puts "ok"

    print "testing identity indirect proxy... "
    STDOUT.flush
    obj.shutdown()
    manager.startServer()
    begin
        obj3 = Test::TestIntfPrx::checkedCast(base3)
        obj3.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    begin
        obj2 = Test::TestIntfPrx::checkedCast(base2)
        obj2.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    obj.shutdown()
    manager.startServer()
    begin
        obj2 = Test::TestIntfPrx::checkedCast(base2)
        obj2.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    begin
        obj3 = Test::TestIntfPrx::checkedCast(base3)
        obj3.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    obj.shutdown()
    manager.startServer()

    begin
        obj2 = Test::TestIntfPrx::checkedCast(base2)
        obj2.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    obj.shutdown()
    manager.startServer()
    begin
        obj3 = Test::TestIntfPrx::checkedCast(base3)
        obj3.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    obj.shutdown()
    manager.startServer()
    begin
        obj2 = Test::TestIntfPrx::checkedCast(base2)
        obj2.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    obj.shutdown()
    manager.startServer()

    begin
        obj5 = Test::TestIntfPrx::checkedCast(base5)
        obj5.ice_ping()
    rescue Ice::LocalException
        test(false)
    end
    puts "ok"

    print "testing reference with unknown identity... "
    STDOUT.flush
    begin
        base = communicator.stringToProxy("unknown/unknown")
        base.ice_ping()
        test(false)
    rescue Ice::NotRegisteredException => ex
        test(ex.kindOfObject == "object")
        test(ex.id == "unknown/unknown")
    end
    puts "ok"

    print "testing reference with unknown adapter... "
    STDOUT.flush
    begin
        base = communicator.stringToProxy("test @ TestAdapterUnknown")
        base.ice_ping()
        test(false)
    rescue Ice::NotRegisteredException => ex
        test(ex.kindOfObject == "object adapter")
        test(ex.id == "TestAdapterUnknown")
    end
    puts "ok"

    print "testing object reference from server... "
    STDOUT.flush
    hello = obj.getHello()
    hello.sayHello()
    puts "ok"

    print "testing object reference from server after shutdown... "
    STDOUT.flush
    obj.shutdown()
    manager.startServer()
    hello.sayHello()
    puts "ok"

    print "testing object migration... "
    STDOUT.flush
    hello = Test::HelloPrx::checkedCast(communicator.stringToProxy("hello"))
    obj.migrateHello()
    hello.sayHello()
    obj.migrateHello()
    hello.sayHello()
    obj.migrateHello()
    hello.sayHello()
    puts "ok"

    print "shutdown server... "
    STDOUT.flush
    obj.shutdown()
    puts "ok"

    print "testing whether server is gone... "
    STDOUT.flush
    begin
        obj2.ice_ping()
        test(false)
    rescue Ice::LocalException
    end
    begin
        obj3.ice_ping()
        test(false)
    rescue Ice::LocalException
    end
    begin
        obj5.ice_ping()
        test(false)
    rescue Ice::LocalException
    end
    puts "ok"

    print "shutdown server manager... "
    STDOUT.flush
    manager.shutdown()
    puts "ok"
end
