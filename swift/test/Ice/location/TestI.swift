// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

final class HelloI: Hello {
    func sayHello(current _: Ice.Current) {}
}

final class TestI: TestIntf {
    let _adapter1: Ice.ObjectAdapter
    let _adapter2: Ice.ObjectAdapter
    let _registry: ServerLocatorRegistry

    init(
        adapter1: Ice.ObjectAdapter,
        adapter2: Ice.ObjectAdapter,
        registry: ServerLocatorRegistry
    ) throws {
        _adapter1 = adapter1
        _adapter2 = adapter2
        _registry = registry
    }

    func shutdown(current _: Ice.Current) {
        _adapter1.getCommunicator().shutdown()
    }

    func getHello(current _: Ice.Current) throws -> HelloPrx? {
        return try uncheckedCast(
            prx: _adapter1.createIndirectProxy(Ice.stringToIdentity("hello")),
            type: HelloPrx.self)
    }

    func getReplicatedHello(current _: Ice.Current) throws -> HelloPrx? {
        return try uncheckedCast(
            prx: _adapter1.createProxy(Ice.stringToIdentity("hello")),
            type: HelloPrx.self)
    }

    func migrateHello(current _: Ice.Current) async throws {
        let id = try Ice.stringToIdentity("hello")
        do {
            try await _registry.addObject(_adapter2.add(servant: _adapter1.remove(id), id: id))
        } catch is Ice.NotRegisteredException {
            try await _registry.addObject(_adapter1.add(servant: _adapter2.remove(id), id: id))
        }
    }
}

actor ServerManagerI: ServerManager {
    let _registry: ServerLocatorRegistry
    let _properties: Ice.Properties
    let _helper: TestHelper
    var _communicators = [Ice.Communicator]()
    var _nextPort: Int32 = 1

    init(registry: ServerLocatorRegistry, properties: Ice.Properties, helper: TestHelper) {
        _registry = registry
        _properties = properties
        _helper = helper

        _properties.setProperty(key: "TestAdapter.AdapterId", value: "TestAdapter")
        _properties.setProperty(key: "TestAdapter.ReplicaGroupId", value: "ReplicatedAdapter")
        _properties.setProperty(key: "TestAdapter2.AdapterId", value: "TestAdapter2")
        _properties.setProperty(key: "Ice.PrintAdapterReady", value: "0")
    }

    func startServer(current _: Ice.Current) async throws {
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
        let properties = _properties.clone()
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
                serverCommunicator.getProperties().setProperty(
                    key: "TestAdapter.Endpoints",
                    value: _helper.getTestEndpoint(properties: properties, num: _nextPort, prot: ""))

                _nextPort += 1
                serverCommunicator.getProperties().setProperty(
                    key: "TestAdapter2.Endpoints",
                    value: _helper.getTestEndpoint(properties: properties, num: _nextPort, prot: ""))

                adapter = try serverCommunicator.createObjectAdapter("TestAdapter")
                adapter2 = try serverCommunicator.createObjectAdapter("TestAdapter2")

                let locator = try serverCommunicator.stringToProxy(
                    "locator:\(_helper.getTestEndpoint(properties: properties, num: 0, prot: ""))")!
                try adapter.setLocator(uncheckedCast(prx: locator, type: Ice.LocatorPrx.self))
                try adapter2.setLocator(uncheckedCast(prx: locator, type: Ice.LocatorPrx.self))

                let object = try TestI(adapter1: adapter, adapter2: adapter2, registry: _registry)
                try await _registry.addObject(adapter.add(servant: HelloI(), id: Ice.stringToIdentity("hello")))

                try await _registry.addObject(adapter.add(servant: object, id: Ice.Identity(name: "test")))
                try await _registry.addObject(adapter.add(servant: object, id: Ice.stringToIdentity("test2")))

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

    func shutdown(current: Ice.Current) {
        for c in _communicators {
            c.destroy()
        }
        _communicators.removeAll()
        current.adapter.getCommunicator().shutdown()
    }
}

actor ServerLocator: TestLocator {
    let _registry: ServerLocatorRegistry
    let _registryPrx: Ice.LocatorRegistryPrx
    var _requestCount: Int32

    init(registry: ServerLocatorRegistry, registryPrx: Ice.LocatorRegistryPrx) {
        _registry = registry
        _registryPrx = registryPrx
        _requestCount = 0
    }

    func findAdapterById(id: String, current: Ice.Current) async throws -> ObjectPrx? {
        _requestCount += 1
        if id == "TestAdapter10" || id == "TestAdapter10-2" {
            precondition(current.encoding == Encoding_1_0)
            return try await _registry.getAdapter("TestAdapter")
        } else {
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            try await Task.sleep(for: .milliseconds(1))
            return try await _registry.getAdapter(id)
        }
    }

    func findObjectById(id: Ice.Identity, current _: Ice.Current) async throws -> ObjectPrx? {
        _requestCount += 1
        // We add a small delay to make sure locator request queuing gets tested when
        // running the test on a fast machine
        try await Task.sleep(for: .milliseconds(1))
        return try await _registry.getObject(id)
    }

    func getRegistry(current _: Ice.Current) -> Ice.LocatorRegistryPrx? {
        return _registryPrx
    }

    func getRequestCount(current _: Ice.Current) -> Int32 {
        return _requestCount
    }
}

actor ServerLocatorRegistry: TestLocatorRegistry {
    private var _adapters = [String: Ice.ObjectPrx]()
    private var _objects = [Ice.Identity: Ice.ObjectPrx]()

    func setAdapterDirectProxy(id: String, proxy: ObjectPrx?, current _: Current) {
        if let obj = proxy {
            self._adapters[id] = obj
        } else {
            self._adapters.removeValue(forKey: id)
        }
    }

    func setReplicatedAdapterDirectProxy(
        adapterId adapter: String,
        replicaGroupId replica: String,
        proxy: Ice.ObjectPrx?,
        current _: Ice.Current
    ) async throws {
        if let obj = proxy {
            _adapters[adapter] = obj
            _adapters[replica] = obj
        } else {
            _adapters.removeValue(forKey: adapter)
            _adapters.removeValue(forKey: replica)
        }
    }

    nonisolated func setServerProcessProxy(id _: String, proxy _: Ice.ProcessPrx?, current _: Ice.Current) {}

    func addObject(_ obj: Ice.ObjectPrx?) {
        _objects[obj!.ice_getIdentity()] = obj
    }

    func addObject(obj: Ice.ObjectPrx?, current _: Ice.Current) {
        addObject(obj)
    }

    func getAdapter(_ id: String) throws -> Ice.ObjectPrx {
        guard let obj = _adapters[id] else {
            throw Ice.AdapterNotFoundException()
        }
        return obj
    }

    func getObject(_ id: Ice.Identity) throws -> Ice.ObjectPrx {
        guard let obj = _objects[id] else {
            throw Ice.ObjectNotFoundException()
        }
        return obj
    }
}
