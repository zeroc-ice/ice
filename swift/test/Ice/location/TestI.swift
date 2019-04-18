//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import PromiseKit
import TestCommon

class HelloI: Hello {
    func sayHello(current: Ice.Current) {}
}

class TestI: TestIntf {

    var _adapter1: Ice.ObjectAdapter
    var _adapter2: Ice.ObjectAdapter
    var _registry: ServerLocatorRegistry

    init(adapter1: Ice.ObjectAdapter,
         adapter2: Ice.ObjectAdapter,
         registry: ServerLocatorRegistry) throws {

        _adapter1 = adapter1
        _adapter2 = adapter2
        _registry = registry

        try _registry.addObject(_adapter1.add(servant: HelloI(), id: Ice.stringToIdentity("hello")))
    }

    func shutdown(current: Ice.Current) throws {
        _adapter1.getCommunicator().shutdown()
    }

    func getHello(current: Ice.Current) throws -> HelloPrx? {
        return try uncheckedCast(prx: _adapter1.createIndirectProxy(Ice.stringToIdentity("hello")),
                                 type: HelloPrx.self)
    }

    func getReplicatedHello(current: Ice.Current) throws -> HelloPrx? {
        return try uncheckedCast(prx: _adapter1.createProxy(Ice.stringToIdentity("hello")),
                                 type: HelloPrx.self)
    }

    func migrateHello(current: Ice.Current) throws {
        let id = try Ice.stringToIdentity("hello")
        do {
            try _registry.addObject(_adapter2.add(servant: _adapter1.remove(id), id: id))
        } catch is Ice.NotRegisteredException {
            try _registry.addObject(_adapter1.add(servant: _adapter2.remove(id), id: id))
        }
    }
}

class ServerManagerI: ServerManager {

    var _registry: ServerLocatorRegistry
    var _helper: TestHelper
    var _communicators = [Ice.Communicator]()
    var _nextPort: Int32 = 1

    init(registry: ServerLocatorRegistry, helper: TestHelper) {
        _registry = registry
        _helper = helper
    }

    func startServer(current: Ice.Current) throws {
        for c in _communicators {
            c.waitForShutdown()
            c.destroy()
        }
        _communicators.removeAll()

        //
        // Simulate a server: create a new communicator and object
        // adapter. The object adapter is started on a system allocated
        // port. The configuration used here contains the Ice.Locator
        // configuration variable. The new object adapter will register
        // its endpoints with the locator and create references containing
        // the adapter id instead of the endpoints.
        //
        var initData = Ice.InitializationData()
        let properties = _helper.communicator().getProperties().clone()
        properties.setProperty(key: "TestAdapter.AdapterId", value: "TestAdapter")
        properties.setProperty(key: "TestAdapter.ReplicaGroupId", value: "ReplicatedAdapter")
        properties.setProperty(key: "TestAdapter2.AdapterId", value: "TestAdapter2")
        initData.properties = properties

        let serverCommunicator = try _helper.initialize(initData)
        _communicators.append(serverCommunicator)

        //
        // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
        // another OA(e.g.: TestAdapter2 is re-activated using port of TestAdapter).
        //
        for i in 1...10 {
            var adapter: Ice.ObjectAdapter!
            var adapter2: Ice.ObjectAdapter!

            do {
                _nextPort += 1
                serverCommunicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                               value: _helper.getTestEndpoint(num: _nextPort))

                _nextPort += 1
                serverCommunicator.getProperties().setProperty(key: "TestAdapter2.Endpoints",
                                                               value: _helper.getTestEndpoint(num: _nextPort))

                adapter = try serverCommunicator.createObjectAdapter("TestAdapter")
                adapter2 = try serverCommunicator.createObjectAdapter("TestAdapter2")

                let locator = try serverCommunicator.stringToProxy("locator:\(_helper.getTestEndpoint(num: 0))")!
                try adapter.setLocator(uncheckedCast(prx: locator, type: Ice.LocatorPrx.self))
                try adapter2.setLocator(uncheckedCast(prx: locator, type: Ice.LocatorPrx.self))

                let object = try TestI(adapter1: adapter, adapter2: adapter2, registry: _registry)
                try _registry.addObject(adapter.add(servant: object, id: Ice.stringToIdentity("test")))
                try _registry.addObject(adapter.add(servant: object, id: Ice.stringToIdentity("test2")))
                _ = try adapter.add(servant: object, id: Ice.stringToIdentity("test3"))

                try adapter.activate()
                try adapter2.activate()
                break
            } catch let ex as Ice.SocketException {
                if i == 10 {
                    throw ex
                }

                // Retry, if OA creation fails with EADDRINUSE(this can occur when running with JS web
                // browser clients if the driver uses ports in the same range as this test, ICE-8148)
                if adapter != nil {
                    adapter.destroy()
                }

                if adapter2 != nil {
                    adapter2.destroy()
                }
            }
        }
    }

    func shutdown(current: Ice.Current) throws {
        for c in _communicators {
            c.destroy()
        }
        _communicators.removeAll()
        current.adapter!.getCommunicator().shutdown()
    }
}

class ServerLocator: TestLocator {

    var _registry: ServerLocatorRegistry
    var _registryPrx: Ice.LocatorRegistryPrx
    var _requestCount: Int32

    init(registry: ServerLocatorRegistry, registryPrx: Ice.LocatorRegistryPrx) {
        _registry = registry
        _registryPrx = registryPrx
        _requestCount = 0
    }

    func findAdapterByIdAsync(id: String, current: Ice.Current) -> Promise<ObjectPrx?> {
        _requestCount += 1
        if id == "TestAdapter10" || id == "TestAdapter10-2" {
            precondition(current.encoding == Ice.Encoding_1_0)
            return Promise<ObjectPrx?> { seal in
                do {
                    try seal.fulfill(_registry.getAdapter("TestAdapter"))
                } catch {
                    seal.reject(error)
                }
            }
        } else {
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            Thread.sleep(forTimeInterval: 0.1)
            return Promise<ObjectPrx?> { seal in
                do {
                    try seal.fulfill(_registry.getAdapter(id))
                } catch {
                    seal.reject(error)
                }
            }
        }
    }

    func findObjectByIdAsync(id: Ice.Identity, current: Ice.Current) -> Promise<ObjectPrx?> {
        _requestCount += 1
        // We add a small delay to make sure locator request queuing gets tested when
        // running the test on a fast machine
        Thread.sleep(forTimeInterval: 0.1)
        return Promise<ObjectPrx?> { seal in
            do {
                try seal.fulfill(_registry.getObject(id))
            } catch {
                seal.reject(error)
            }
        }
    }

    func getRegistry(current: Ice.Current) throws -> Ice.LocatorRegistryPrx? {
        return _registryPrx
    }

    func getRequestCount(current: Ice.Current) throws -> Int32 {
        return _requestCount
    }
}
