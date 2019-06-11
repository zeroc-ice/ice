//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit
import Foundation

func allTests(_ helper: TestHelper) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    let manager = try checkedCast(
        prx: communicator.stringToProxy("ServerManager:\(helper.getTestEndpoint(num: 0))")!,
        type: ServerManagerPrx.self)!

    let locator = uncheckedCast(prx: communicator.getDefaultLocator()!, type: TestLocatorPrx.self)

    let registry = try checkedCast(prx: locator.getRegistry()!, type: TestLocatorRegistryPrx.self)!

    output.write("testing stringToProxy... ")
    var base = try communicator.stringToProxy("test @ TestAdapter")!
    let base2 = try communicator.stringToProxy("test @ TestAdapter")!
    let base3 = try communicator.stringToProxy("test")!
    let base4 = try communicator.stringToProxy("ServerManager")!
    let base5 = try communicator.stringToProxy("test2")!
    let base6 = try communicator.stringToProxy("test @ ReplicatedAdapter")!
    output.writeLine("ok")

    output.write("testing ice_locator and ice_getLocator... ")
    try test(base.ice_getLocator()!.ice_getIdentity() == communicator.getDefaultLocator()!.ice_getIdentity())
    let anotherLocator = try uncheckedCast(prx: communicator.stringToProxy("anotherLocator")!,
                                           type: Ice.LocatorPrx.self)
    base = base.ice_locator(anotherLocator)
    try test(base.ice_getLocator()!.ice_getIdentity() == anotherLocator.ice_getIdentity())
    communicator.setDefaultLocator(nil)

    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(base.ice_getLocator() == nil)
    base = base.ice_locator(anotherLocator)
    try test(base.ice_getLocator()!.ice_getIdentity() == anotherLocator.ice_getIdentity())
    communicator.setDefaultLocator(locator)
    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(base.ice_getLocator()!.ice_getIdentity() == communicator.getDefaultLocator()!.ice_getIdentity())

    //
    // We also test ice_router/ice_getRouter(perhaps we should add a
    // test/Ice/router test?)
    //
    try test(base.ice_getRouter() == nil)
    let anotherRouter = try uncheckedCast(prx: communicator.stringToProxy("anotherRouter")!, type: Ice.RouterPrx.self)
    base = base.ice_router(anotherRouter)
    try test(base.ice_getRouter()!.ice_getIdentity() == anotherRouter.ice_getIdentity())
    let router = try uncheckedCast(prx: communicator.stringToProxy("dummyrouter")!, type: Ice.RouterPrx.self)
    communicator.setDefaultRouter(router)
    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(base.ice_getRouter()!.ice_getIdentity() == communicator.getDefaultRouter()!.ice_getIdentity())
    communicator.setDefaultRouter(nil)
    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(base.ice_getRouter() == nil)
    output.writeLine("ok")

    output.write("starting server... ")
    try manager.startServer()
    output.writeLine("ok")

    output.write("testing checked cast... ")
    var obj = try checkedCast(prx: base, type: TestIntfPrx.self)!
    let obj2 = try checkedCast(prx: base2, type: TestIntfPrx.self)!
    let obj3 = try checkedCast(prx: base3, type: TestIntfPrx.self)!
    _ = try checkedCast(prx: base4, type: ServerManagerPrx.self)!
    let obj5 = try checkedCast(prx: base5, type: TestIntfPrx.self)!
    let obj6 = try checkedCast(prx: base6, type: TestIntfPrx.self)!
    output.writeLine("ok")

    output.write("testing id@AdapterId indirect proxy... ")
    try obj.shutdown()
    try manager.startServer()
    do {
        try obj2.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing id@ReplicaGroupId indirect proxy... ")
    try obj.shutdown()
    try manager.startServer()
    do {
        try obj6.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing identity indirect proxy... ")
    try obj.shutdown()
    try manager.startServer()

    do {
        try obj3.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try obj2.ice_ping()
    } catch {
        try test(false)
    }
    try obj.shutdown()
    try manager.startServer()

    do {
        try obj2.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try obj3.ice_ping()
    } catch {
        try test(false)
    }

    try obj.shutdown()
    try manager.startServer()

    do {
        try obj2.ice_ping()
    } catch {
        try test(false)
    }

    try obj.shutdown()
    try manager.startServer()

    do {
        try obj3.ice_ping()
    } catch {
        try test(false)
    }

    try obj.shutdown()
    try manager.startServer()

    do {
        let obj5 = try checkedCast(prx: base5, type: TestIntfPrx.self)!
        try obj5.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing proxy with unknown identity... ")
    do {
        base = try communicator.stringToProxy("unknown/unknown")!
        try base.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object")
        try test(ex.id == "unknown/unknown")
    }
    output.writeLine("ok")

    output.write("testing proxy with unknown adapter... ")
    do {
        base = try communicator.stringToProxy("test @ TestAdapterUnknown")!
        try base.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object adapter")
        try test(ex.id == "TestAdapterUnknown")
    }
    output.writeLine("ok")

    output.write("testing locator cache timeout... ")
    let basencc = try communicator.stringToProxy("test@TestAdapter")!.ice_connectionCached(false)
    var count = try locator.getRequestCount()
    try basencc.ice_locatorCacheTimeout(0).ice_ping() // No locator cache.
    count += 1
    try test(count == locator.getRequestCount())
    try basencc.ice_locatorCacheTimeout(0).ice_ping() // No locator cache.
    count += 1
    try test(count == locator.getRequestCount())
    try basencc.ice_locatorCacheTimeout(2).ice_ping() // 2s timeout.
    try test(count == locator.getRequestCount())
    Thread.sleep(forTimeInterval: 1.3) // 1300ms
    try basencc.ice_locatorCacheTimeout(1).ice_ping() // 1s timeout.
    count += 1
    try test(count == locator.getRequestCount())

    try communicator.stringToProxy("test")!.ice_locatorCacheTimeout(0).ice_ping() // No locator cache.
    count += 2
    try test(count == locator.getRequestCount())
    try communicator.stringToProxy("test")!.ice_locatorCacheTimeout(2).ice_ping() // 2s timeout
    try test(count == locator.getRequestCount())
    Thread.sleep(forTimeInterval: 1.3) // 1300ms
    try communicator.stringToProxy("test")!.ice_locatorCacheTimeout(1).ice_ping() // 1s timeout
    count += 2
    try test(count == locator.getRequestCount())

    try communicator.stringToProxy("test@TestAdapter")!.ice_locatorCacheTimeout(-1).ice_ping()
    try test(count == locator.getRequestCount())
    try communicator.stringToProxy("test")!.ice_locatorCacheTimeout(-1).ice_ping()
    try test(count == locator.getRequestCount())
    try communicator.stringToProxy("test@TestAdapter")!.ice_ping()
    try test(count == locator.getRequestCount())
    try communicator.stringToProxy("test")!.ice_ping()
    try test(count == locator.getRequestCount())

    try test(communicator.stringToProxy("test")!.ice_locatorCacheTimeout(99).ice_getLocatorCacheTimeout() == 99)

    output.writeLine("ok")

    output.write("testing proxy from server... ")
    obj = try checkedCast(prx: communicator.stringToProxy("test@TestAdapter")!, type: TestIntfPrx.self)!
    var hello = try obj.getHello()!
    try test(hello.ice_getAdapterId() == "TestAdapter")
    try hello.sayHello()
    hello = try obj.getReplicatedHello()!
    try test(hello.ice_getAdapterId() == "ReplicatedAdapter")
    try hello.sayHello()
    output.writeLine("ok")

    output.write("testing locator request queuing... ")
    hello = try obj.getReplicatedHello()!.ice_locatorCacheTimeout(0).ice_connectionCached(false)
    count = try locator.getRequestCount()
    try hello.ice_ping()
    count += 1
    try test(count == locator.getRequestCount())

    var results = [Promise<Void>](repeating: hello.sayHelloAsync(), count: 1000)
    for r in results {
        try r.wait()
    }
    results.removeAll()
    try test(locator.getRequestCount() > count &&
             locator.getRequestCount() < count + 999)

    if try locator.getRequestCount() > count + 800 {
        try output.write("queuing = \(locator.getRequestCount() - count)")
    }

    count = try locator.getRequestCount()
    hello = hello.ice_adapterId("unknown")

    results = [Promise<Void>](repeating: hello.sayHelloAsync(), count: 1000)
    for r in results {
        do {
            try r.wait()
            try test(false)
        } catch is Ice.NotRegisteredException {}
    }
    results.removeAll()
    // XXX:
    // Take into account the retries.
    try test(locator.getRequestCount() > count &&
             locator.getRequestCount() < count + 1999)

    if try locator.getRequestCount() > count + 800 {
        try output.write("queuing = \(locator.getRequestCount() - count)")
    }
    output.writeLine("ok")

    output.write("testing adapter locator cache... ")
    do {
        try communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object adapter")
        try test(ex.id == "TestAdapter3")
    }

    try registry.setAdapterDirectProxy(id: "TestAdapter3", proxy: locator.findAdapterById("TestAdapter"))
    do {
        try communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
        try registry.setAdapterDirectProxy(
            id: "TestAdapter3",
            proxy: communicator.stringToProxy("dummy:\(helper.getTestEndpoint(num: 99))"))
        try communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try communicator.stringToProxy("test@TestAdapter3")!.ice_locatorCacheTimeout(0).ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    try registry.setAdapterDirectProxy(id: "TestAdapter3", proxy: locator.findAdapterById("TestAdapter"))
    do {
        try communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing well-known object locator cache... ")
    try registry.addObject(communicator.stringToProxy("test3@TestUnknown"))
    do {
        try communicator.stringToProxy("test3")!.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object adapter")
        try test(ex.id == "TestUnknown")
    }
    try registry.addObject(communicator.stringToProxy("test3@TestAdapter4")) // Update
    try registry.setAdapterDirectProxy(id: "TestAdapter4",
                                       proxy: communicator.stringToProxy("dummy:\(helper.getTestEndpoint(num: 99))"))
    do {
        try communicator.stringToProxy("test3")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    try registry.setAdapterDirectProxy(id: "TestAdapter4", proxy: locator.findAdapterById("TestAdapter"))
    do {
        try communicator.stringToProxy("test3")!.ice_ping()
    } catch {
        try test(false)
    }

    try registry.setAdapterDirectProxy(id: "TestAdapter4",
                                       proxy: communicator.stringToProxy("dummy:\(helper.getTestEndpoint(num: 99))"))
    do {
        try communicator.stringToProxy("test3")!.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try communicator.stringToProxy("test@TestAdapter4")!.ice_locatorCacheTimeout(0).ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try communicator.stringToProxy("test@TestAdapter4")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try communicator.stringToProxy("test3")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    try registry.addObject(communicator.stringToProxy("test3@TestAdapter"))

    do {
        try communicator.stringToProxy("test3")!.ice_ping()
    } catch {
        try test(false)
    }

    try registry.addObject(communicator.stringToProxy("test4"))

    do {
        try communicator.stringToProxy("test4")!.ice_ping()
        try test(false)
    } catch is Ice.NoEndpointException {}
    output.writeLine("ok")

    output.write("testing locator cache background updates... ")
    do {
        let properties = communicator.getProperties().clone()
        properties.setProperty(key: "Ice.BackgroundLocatorCacheUpdates", value: "1")
        var initData = Ice.InitializationData()
        initData.properties = properties

        let ic = try helper.initialize(initData)

        try registry.setAdapterDirectProxy(id: "TestAdapter5", proxy: locator.findAdapterById("TestAdapter"))
        try registry.addObject(communicator.stringToProxy("test3@TestAdapter"))

        count = try locator.getRequestCount()
        try ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(0).ice_ping() // No locator cache.
        try ic.stringToProxy("test3")!.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        count += 3
        try test(count == locator.getRequestCount())
        try registry.setAdapterDirectProxy(id: "TestAdapter5", proxy: nil)
        try registry.addObject(communicator.stringToProxy("test3:" + helper.getTestEndpoint(num: 99)))
        try ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
        try ic.stringToProxy("test3")!.ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
        try test(count == locator.getRequestCount())
        Thread.sleep(forTimeInterval: 1.2)

        // The following request should trigger the background
        // updates but still use the cached endpoints and
        // therefore succeed.
        try ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(1).ice_ping() // 1s timeout.
        try ic.stringToProxy("test3")!.ice_locatorCacheTimeout(1).ice_ping() // 1s timeout.

        do {
            while true {
                try ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(1).ice_ping() // 1s timeout.
                Thread.sleep(forTimeInterval: 0.1)
            }
        } catch is Ice.LocalException {
            // Expected to fail once they endpoints have been updated in the background.
        }

        do {
            while true {
                try ic.stringToProxy("test3")!.ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                Thread.sleep(forTimeInterval: 0.1)
            }
        } catch is Ice.LocalException {
            // Expected to fail once they endpoints have been updated in the background.
        }
        ic.destroy()
    }
    output.writeLine("ok")

    output.write("testing proxy from server after shutdown... ")
    hello = try obj.getReplicatedHello()!
    try obj.shutdown()
    try manager.startServer()
    try hello.sayHello()
    output.writeLine("ok")

    output.write("testing object migration... ")
    hello = try checkedCast(prx: communicator.stringToProxy("hello")!, type: HelloPrx.self)!
    try obj.migrateHello()
    try hello.ice_getConnection()!.close(.GracefullyWithWait)
    try hello.sayHello()
    try obj.migrateHello()
    try hello.sayHello()
    try obj.migrateHello()
    try hello.sayHello()
    output.writeLine("ok")

    output.write("testing locator encoding resolution... ")
    hello = try checkedCast(prx: communicator.stringToProxy("hello")!, type: HelloPrx.self)!
    count = try locator.getRequestCount()
    try communicator.stringToProxy("test@TestAdapter")!.ice_encodingVersion(Ice.Encoding_1_1).ice_ping()
    try test(count == locator.getRequestCount())
    try communicator.stringToProxy("test@TestAdapter10")!.ice_encodingVersion(Ice.Encoding_1_0).ice_ping()
    count += 1
    try test(count == locator.getRequestCount())
    try communicator.stringToProxy("test -e 1.0@TestAdapter10-2")!.ice_ping()
    count += 1
    try test(count == locator.getRequestCount())
    output.writeLine("ok")

    output.write("shutdown server... ")
    try obj.shutdown()
    output.writeLine("ok")

    output.write("testing whether server is gone... ")
    do {
        try obj2.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try obj3.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try obj5.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}
    output.writeLine("ok")

    output.write("testing indirect proxies to collocated objects... ")

    //
    // Set up test for calling a collocated object through an
    // indirect, adapterless reference.
    //
    let properties = communicator.getProperties()
    properties.setProperty(key: "Ice.PrintAdapterReady", value: "0")
    let adapter = try communicator.createObjectAdapterWithEndpoints(name: "Hello", endpoints: "tcp -h *")
    try adapter.setLocator(locator)

    var ident = Ice.Identity()
    ident.name = UUID().uuidString
    try registry.addObject(adapter.add(servant: HelloDisp(HelloI()), id: ident))
    try adapter.activate()

    /*let helloPrx*/ _ = try checkedCast(
        prx: communicator.stringToProxy("\"\(communicator.identityToString(ident))\"")!,
        type: HelloPrx.self)!

    // TODO in Swift the call doesn't use collocation optimization because
    // ServantManager::hasServant only checks C++ ASM for the given identity
    // try test(helloPrx.ice_getConnection() == nil)

    adapter.deactivate()
    output.writeLine("ok")

    output.write("shutdown server manager... ")
    try manager.shutdown()
    output.writeLine("ok")
}
