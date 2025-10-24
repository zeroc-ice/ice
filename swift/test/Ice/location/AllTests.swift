// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

func allTests(_ helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let output = helper.getWriter()

    let communicator = helper.communicator()

    let manager = try makeProxy(
        communicator: communicator,
        proxyString: "ServerManager:\(helper.getTestEndpoint(num: 0))",
        type: ServerManagerPrx.self
    )

    let locator = uncheckedCast(prx: communicator.getDefaultLocator()!, type: TestLocatorPrx.self)
    let registry = try await uncheckedCast(
        prx: locator.getRegistry()!, type: TestLocatorRegistryPrx.self)

    output.write("testing stringToProxy... ")
    var base = try communicator.stringToProxy("test @ TestAdapter")!
    let base2 = try communicator.stringToProxy("test @ TestAdapter")!
    let base3 = try communicator.stringToProxy("test")!
    let base4 = try communicator.stringToProxy("ServerManager")!
    let base5 = try communicator.stringToProxy("test2")!
    let base6 = try communicator.stringToProxy("test @ ReplicatedAdapter")!
    output.writeLine("ok")

    output.write("testing ice_locator and ice_getLocator... ")
    try test(
        base.ice_getLocator()!.ice_getIdentity()
            == communicator.getDefaultLocator()!.ice_getIdentity())
    let anotherLocator = try makeProxy(
        communicator: communicator, proxyString: "anotherLocator", type: Ice.LocatorPrx.self)
    base = base.ice_locator(anotherLocator)
    try test(base.ice_getLocator()!.ice_getIdentity() == anotherLocator.ice_getIdentity())
    communicator.setDefaultLocator(nil)

    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(base.ice_getLocator() == nil)
    base = base.ice_locator(anotherLocator)
    try test(base.ice_getLocator()!.ice_getIdentity() == anotherLocator.ice_getIdentity())
    communicator.setDefaultLocator(locator)
    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(
        base.ice_getLocator()!.ice_getIdentity()
            == communicator.getDefaultLocator()!.ice_getIdentity())

    //
    // We also test ice_router/ice_getRouter(perhaps we should add a
    // test/Ice/router test?)
    //
    try test(base.ice_getRouter() == nil)
    let anotherRouter = try makeProxy(
        communicator: communicator, proxyString: "anotherrouter", type: Ice.RouterPrx.self)
    base = base.ice_router(anotherRouter)
    try test(base.ice_getRouter()!.ice_getIdentity() == anotherRouter.ice_getIdentity())
    let router = try makeProxy(
        communicator: communicator, proxyString: "dummyrouter", type: Ice.RouterPrx.self)
    communicator.setDefaultRouter(router)
    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(
        base.ice_getRouter()!.ice_getIdentity()
            == communicator.getDefaultRouter()!.ice_getIdentity())
    communicator.setDefaultRouter(nil)
    base = try communicator.stringToProxy("test @ TestAdapter")!
    try test(base.ice_getRouter() == nil)
    output.writeLine("ok")

    output.write("starting server... ")
    try await manager.startServer()
    output.writeLine("ok")

    output.write("testing checked cast... ")
    var obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    let obj2 = try await checkedCast(prx: base2, type: TestIntfPrx.self)!
    let obj3 = try await checkedCast(prx: base3, type: TestIntfPrx.self)!
    _ = try await checkedCast(prx: base4, type: ServerManagerPrx.self)!
    let obj5 = try await checkedCast(prx: base5, type: TestIntfPrx.self)!
    let obj6 = try await checkedCast(prx: base6, type: TestIntfPrx.self)!
    output.writeLine("ok")

    output.write("testing id@AdapterId indirect proxy... ")
    try await obj.shutdown()
    try await manager.startServer()
    do {
        try await obj2.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing id@ReplicaGroupId indirect proxy... ")
    try await obj.shutdown()
    try await manager.startServer()
    do {
        try await obj6.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing identity indirect proxy... ")
    try await obj.shutdown()
    try await manager.startServer()

    do {
        try await obj3.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try await obj2.ice_ping()
    } catch {
        try test(false)
    }
    try await obj.shutdown()
    try await manager.startServer()

    do {
        try await obj2.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try await obj3.ice_ping()
    } catch {
        try test(false)
    }

    try await obj.shutdown()
    try await manager.startServer()

    do {
        try await obj2.ice_ping()
    } catch {
        try test(false)
    }

    try await obj.shutdown()
    try await manager.startServer()

    do {
        try await obj3.ice_ping()
    } catch {
        try test(false)
    }

    try await obj.shutdown()
    try await manager.startServer()

    do {
        let obj5 = try await checkedCast(prx: base5, type: TestIntfPrx.self)!
        try await obj5.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing proxy with unknown identity... ")
    do {
        base = try communicator.stringToProxy("unknown/unknown")!
        try await base.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object")
        try test(ex.id == "unknown/unknown")
    }
    output.writeLine("ok")

    output.write("testing proxy with unknown adapter... ")
    do {
        base = try communicator.stringToProxy("test @ TestAdapterUnknown")!
        try await base.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object adapter")
        try test(ex.id == "TestAdapterUnknown")
    }
    output.writeLine("ok")

    output.write("testing locator cache timeout... ")
    let basencc = try communicator.stringToProxy("test@TestAdapter")!.ice_connectionCached(false)
    var count = try await locator.getRequestCount()
    try await basencc.ice_locatorCacheTimeout(0).ice_ping()  // No locator cache.
    count += 1
    try await test(count == locator.getRequestCount())
    try await basencc.ice_locatorCacheTimeout(0).ice_ping()  // No locator cache.
    count += 1
    try await test(count == locator.getRequestCount())
    try await basencc.ice_locatorCacheTimeout(2).ice_ping()  // 2s timeout.
    try await test(count == locator.getRequestCount())
    try await Task.sleep(for: .milliseconds(1300))
    try await basencc.ice_locatorCacheTimeout(1).ice_ping()  // 1s timeout.
    count += 1
    try await test(count == locator.getRequestCount())

    // No locator cache.
    try await communicator.stringToProxy("test")!.ice_locatorCacheTimeout(0).ice_ping()
    count += 2
    try await test(count == locator.getRequestCount())
    try await communicator.stringToProxy("test")!.ice_locatorCacheTimeout(2).ice_ping()  // 2s timeout
    try await test(count == locator.getRequestCount())
    try await Task.sleep(for: .milliseconds(1300))
    try await communicator.stringToProxy("test")!.ice_locatorCacheTimeout(1).ice_ping()  // 1s timeout
    count += 2
    try await test(count == locator.getRequestCount())

    try await communicator.stringToProxy("test@TestAdapter")!.ice_locatorCacheTimeout(-1).ice_ping()
    try await test(count == locator.getRequestCount())
    try await communicator.stringToProxy("test")!.ice_locatorCacheTimeout(-1).ice_ping()
    try await test(count == locator.getRequestCount())
    try await communicator.stringToProxy("test@TestAdapter")!.ice_ping()
    try await test(count == locator.getRequestCount())
    try await communicator.stringToProxy("test")!.ice_ping()
    try await test(count == locator.getRequestCount())

    try test(
        communicator.stringToProxy("test")!.ice_locatorCacheTimeout(99).ice_getLocatorCacheTimeout()
            == 99)

    output.writeLine("ok")

    output.write("testing proxy from server... ")
    obj = try makeProxy(
        communicator: communicator, proxyString: "test@TestAdapter", type: TestIntfPrx.self)
    var hello = try await obj.getHello()!
    try test(hello.ice_getAdapterId() == "TestAdapter")
    try await hello.sayHello()
    hello = try await obj.getReplicatedHello()!
    try test(hello.ice_getAdapterId() == "ReplicatedAdapter")
    try await hello.sayHello()
    output.writeLine("ok")

    output.write("testing locator request queuing... ")
    hello = try await obj.getReplicatedHello()!.ice_locatorCacheTimeout(0).ice_connectionCached(
        false)
    count = try await locator.getRequestCount()
    try await hello.ice_ping()
    count += 1
    try await test(count == locator.getRequestCount())

    try await withThrowingTaskGroup(of: Void.self) { [hello] taskGroup in
        for _ in 0..<1000 {
            taskGroup.addTask {
                try await hello.sayHello()
            }
        }

        // wait for all tasks
        try await taskGroup.waitForAll()
    }

    try await test(count...count + 1999 ~= locator.getRequestCount())

    if try await locator.getRequestCount() > count + 800 {
        try await output.write("queuing = \(locator.getRequestCount() - count)")
    }

    count = try await locator.getRequestCount()
    hello = hello.ice_adapterId("unknown")

    try await withThrowingTaskGroup(of: Void.self) { [hello] taskGroup in
        for _ in 0..<1000 {
            taskGroup.addTask {
                do {
                    try await hello.sayHello()
                    try test(false)
                } catch is Ice.NotRegisteredException {}
            }
        }

        // wait for all tasks
        try await taskGroup.waitForAll()
    }

    // TODO: Take into account the retries.
    try await test(count...count + 1999 ~= locator.getRequestCount())

    if try await locator.getRequestCount() > count + 800 {
        try await output.write("queuing = \(locator.getRequestCount() - count)")
    }
    output.writeLine("ok")

    output.write("testing adapter locator cache... ")
    do {
        try await communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object adapter")
        try test(ex.id == "TestAdapter3")
    }

    try await registry.setAdapterDirectProxy(
        id: "TestAdapter3", proxy: locator.findAdapterById("TestAdapter"))
    do {
        try await communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
        try await registry.setAdapterDirectProxy(
            id: "TestAdapter3",
            proxy: communicator.stringToProxy("dummy:\(helper.getTestEndpoint(num: 99))")
        )
        try await communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try await communicator.stringToProxy("test@TestAdapter3")!.ice_locatorCacheTimeout(0)
            .ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try await communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    try await registry.setAdapterDirectProxy(
        id: "TestAdapter3", proxy: locator.findAdapterById("TestAdapter"))
    do {
        try await communicator.stringToProxy("test@TestAdapter3")!.ice_ping()
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("testing well-known object locator cache... ")
    try await registry.addObject(communicator.stringToProxy("test3@TestUnknown"))
    do {
        try await communicator.stringToProxy("test3")!.ice_ping()
        try test(false)
    } catch let ex as Ice.NotRegisteredException {
        try test(ex.kindOfObject == "object adapter")
        try test(ex.id == "TestUnknown")
    }
    try await registry.addObject(communicator.stringToProxy("test3@TestAdapter4"))  // Update
    try await registry.setAdapterDirectProxy(
        id: "TestAdapter4",
        proxy: communicator.stringToProxy("dummy:\(helper.getTestEndpoint(num: 99))"))
    do {
        try await communicator.stringToProxy("test3")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    try await registry.setAdapterDirectProxy(
        id: "TestAdapter4", proxy: locator.findAdapterById("TestAdapter"))
    do {
        try await communicator.stringToProxy("test3")!.ice_ping()
    } catch {
        try test(false)
    }

    try await registry.setAdapterDirectProxy(
        id: "TestAdapter4",
        proxy: communicator.stringToProxy("dummy:\(helper.getTestEndpoint(num: 99))"))
    do {
        try await communicator.stringToProxy("test3")!.ice_ping()
    } catch {
        try test(false)
    }

    do {
        try await communicator.stringToProxy("test@TestAdapter4")!.ice_locatorCacheTimeout(0)
            .ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try await communicator.stringToProxy("test@TestAdapter4")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try await communicator.stringToProxy("test3")!.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    try await registry.addObject(communicator.stringToProxy("test3@TestAdapter"))

    do {
        try await communicator.stringToProxy("test3")!.ice_ping()
    } catch {
        try test(false)
    }

    try await registry.addObject(communicator.stringToProxy("test4"))

    do {
        try await communicator.stringToProxy("test4")!.ice_ping()
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

        try await registry.setAdapterDirectProxy(
            id: "TestAdapter5", proxy: locator.findAdapterById("TestAdapter"))
        try await registry.addObject(communicator.stringToProxy("test3@TestAdapter"))

        count = try await locator.getRequestCount()

        // No locator cache.
        try await ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(0).ice_ping()
        try await ic.stringToProxy("test3")!.ice_locatorCacheTimeout(0).ice_ping()  // No locator cache.
        count += 3
        try await test(count == locator.getRequestCount())
        try await registry.setAdapterDirectProxy(id: "TestAdapter5", proxy: nil)
        try await registry.addObject(
            communicator.stringToProxy("test3:" + helper.getTestEndpoint(num: 99)))

        // 10s timeout.
        try await ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(10).ice_ping()
        try await ic.stringToProxy("test3")!.ice_locatorCacheTimeout(10).ice_ping()  // 10s timeout.
        try await test(count == locator.getRequestCount())
        try await Task.sleep(for: .milliseconds(1200))

        // The following request should trigger the background
        // updates but still use the cached endpoints and
        // therefore succeed.

        // 1s timeout.
        try await ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(1).ice_ping()
        try await ic.stringToProxy("test3")!.ice_locatorCacheTimeout(1).ice_ping()  // 1s timeout.

        do {
            while true {
                // 1s timeout.
                try await ic.stringToProxy("test@TestAdapter5")!.ice_locatorCacheTimeout(1)
                    .ice_ping()
                try await Task.sleep(for: .milliseconds(10))
            }
        } catch is Ice.LocalException {
            // Expected to fail once they endpoints have been updated in the background.
        }

        do {
            while true {
                try await ic.stringToProxy("test3")!.ice_locatorCacheTimeout(1).ice_ping()  // 1s timeout.
                try await Task.sleep(for: .milliseconds(10))
            }
        } catch is Ice.LocalException {
            // Expected to fail once they endpoints have been updated in the background.
        }
        ic.destroy()
    }
    output.writeLine("ok")

    output.write("testing proxy from server after shutdown... ")
    hello = try await obj.getReplicatedHello()!
    try await obj.shutdown()
    try await manager.startServer()
    try await hello.sayHello()
    output.writeLine("ok")

    output.write("testing object migration... ")
    hello = try makeProxy(communicator: communicator, proxyString: "hello", type: HelloPrx.self)
    try await obj.migrateHello()
    try await hello.ice_getConnection()!.close()
    try await hello.sayHello()
    try await obj.migrateHello()
    try await hello.sayHello()
    try await obj.migrateHello()
    try await hello.sayHello()
    output.writeLine("ok")

    output.write("testing locator encoding resolution... ")
    hello = try makeProxy(communicator: communicator, proxyString: "hello", type: HelloPrx.self)
    count = try await locator.getRequestCount()
    try await communicator.stringToProxy("test@TestAdapter")!.ice_encodingVersion(Ice.Encoding_1_1)
        .ice_ping()
    try await test(count == locator.getRequestCount())
    try await communicator.stringToProxy("test@TestAdapter10")!.ice_encodingVersion(
        Ice.Encoding_1_0
    )
    .ice_ping()
    count += 1
    try await test(count == locator.getRequestCount())
    try await communicator.stringToProxy("test -e 1.0@TestAdapter10-2")!.ice_ping()
    count += 1
    try await test(count == locator.getRequestCount())
    output.writeLine("ok")

    output.write("shutdown server... ")
    try await obj.shutdown()
    output.writeLine("ok")

    output.write("testing whether server is gone... ")
    do {
        try await obj2.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try await obj3.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try await obj5.ice_ping()
        try test(false)
    } catch is Ice.LocalException {}
    output.writeLine("ok")

    output.write("testing indirect proxies to collocated objects... ")

    //
    // Set up test for calling a collocated object through an
    // indirect, adapter-less reference.
    //
    communicator.getProperties().setProperty(key: "Hello.AdapterId", value: UUID().uuidString)
    let adapter = try communicator.createObjectAdapterWithEndpoints(
        name: "Hello", endpoints: "default")

    var ident = Ice.Identity()
    ident.name = UUID().uuidString
    try adapter.add(servant: HelloI(), id: ident)

    do {
        let helloPrx = try makeProxy(
            communicator: communicator, proxyString: "\(communicator.identityToString(ident))",
            type: HelloPrx.self)
        try await helloPrx.ice_ping()
        try test(false)
    } catch is Ice.NotRegisteredException {
        // Calls on the well-known proxy are not collocated because of issue #507
    }

    // Ensure that calls on the indirect proxy (with adapter ID) is collocated
    var helloPrx = try await checkedCast(
        prx: adapter.createIndirectProxy(ident), type: HelloPrx.self)!
    try await test(helloPrx.ice_getConnection() == nil)

    // Ensure that calls on the direct proxy is collocated
    helloPrx = try await checkedCast(prx: adapter.createDirectProxy(ident), type: HelloPrx.self)!
    try await test(helloPrx.ice_getConnection() == nil)

    output.writeLine("ok")

    output.write("shutdown server manager... ")
    try await manager.shutdown()
    output.writeLine("ok")
}
