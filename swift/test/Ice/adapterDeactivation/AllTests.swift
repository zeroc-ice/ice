// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

func allTests(_ helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()
    output.write("testing stringToProxy... ")
    let ref = "test:\(helper.getTestEndpoint(num: 0))"
    let base = try communicator.stringToProxy(ref)!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    let obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    try test(obj == base)
    output.writeLine("ok")

    do {
        output.write("creating/destroying/recreating object adapter... ")
        var adapter = try communicator.createObjectAdapterWithEndpoints(
            name: "TransientTestAdapter",
            endpoints: "default")
        do {
            _ = try communicator.createObjectAdapterWithEndpoints(
                name: "TransientTestAdapter", endpoints: "default")
            try test(false)
        } catch is Ice.AlreadyRegisteredException {}
        adapter.destroy()

        //
        // Use a different port than the first adapter to avoid an "address already in use" error.
        //
        adapter = try communicator.createObjectAdapterWithEndpoints(
            name: "TransientTestAdapter", endpoints: "default")
        adapter.destroy()
        output.writeLine("ok")
    }

    output.write("creating/activating/deactivating object adapter in one operation... ")
    try await obj.transient()
    try await obj.transient()
    output.writeLine("ok")

    output.write("testing object adapter deactivation... ")
    do {
        let adapter = try communicator.createObjectAdapterWithEndpoints(
            name: "TransientTestAdapter",
            endpoints: "default")
        try adapter.activate()
        try test(!adapter.isDeactivated())
        adapter.deactivate()
        try test(adapter.isDeactivated())
        adapter.destroy()
    }
    output.writeLine("ok")

    do {
        output.write("testing connection closure... ")
        for _ in 0..<10 {
            let comm = try Ice.initialize(Ice.InitializationData(properties: communicator.getProperties().clone()))

            // stringToProxy must be called before the communicator is destroyed
            let prx = try comm.stringToProxy(ref)!

            Task {
                try await prx.ice_ping()
            }
            comm.destroy()

        }
        output.writeLine("ok")
    }

    output.write("testing object adapter published endpoints... ")
    do {
        communicator.getProperties().setProperty(
            key: "PAdapter.PublishedEndpoints",
            value: "tcp -h localhost -p 12345 -t 30000")
        let adapter = try communicator.createObjectAdapter("PAdapter")
        try test(adapter.getPublishedEndpoints().count == 1)
        let endpt = adapter.getPublishedEndpoints()[0]
        try test(endpt.toString() == "tcp -h localhost -p 12345 -t 30000")
        let prx = try communicator.stringToProxy(
            "dummy:tcp -h localhost -p 12346 -t 20000:" + "tcp -h localhost -p 12347 -t 10000")!
        try adapter.setPublishedEndpoints(prx.ice_getEndpoints())
        try test(adapter.getPublishedEndpoints().count == 2)
        var id = Ice.Identity()
        id.name = "dummy"

        try test(
            adapter.createProxy(id).ice_getEndpoints().elementsEqual(prx.ice_getEndpoints()) { $0 == $1 })
        try test(adapter.getPublishedEndpoints().elementsEqual(prx.ice_getEndpoints()) { $0 == $1 })
        adapter.destroy()
        try test(adapter.getPublishedEndpoints().count == 0)
    }
    output.writeLine("ok")

    if try await obj.ice_getConnection() != nil {
        output.write("testing object adapter with bi-dir connection... ")
        try test(communicator.getDefaultObjectAdapter() == nil)
        try test(obj.ice_getCachedConnection()!.getAdapter() == nil)

        let adapter = try communicator.createObjectAdapter("")

        communicator.setDefaultObjectAdapter(adapter)
        try test(communicator.getDefaultObjectAdapter() === adapter)

        // create new connection
        try await obj.ice_getCachedConnection()!.close()
        try await obj.ice_ping()

        try test(obj.ice_getCachedConnection()!.getAdapter() === adapter)
        communicator.setDefaultObjectAdapter(nil)

        // create new connection
        try await obj.ice_getCachedConnection()!.close()
        try await obj.ice_ping()

        try test(obj.ice_getCachedConnection()!.getAdapter() == nil)
        try obj.ice_getCachedConnection()!.setAdapter(adapter)
        try test(obj.ice_getCachedConnection()!.getAdapter() === adapter)
        try obj.ice_getCachedConnection()!.setAdapter(nil)

        adapter.destroy()
        do {
            try obj.ice_getCachedConnection()!.setAdapter(adapter)
            try test(false)
        } catch is Ice.ObjectAdapterDestroyedException {}
        output.writeLine("ok")
    }

    output.write("testing object adapter with router... ")
    do {
        var routerId = Ice.Identity()
        routerId.name = "router"
        var router = uncheckedCast(
            prx: base.ice_identity(routerId).ice_connectionId("rc"),
            type: Ice.RouterPrx.self)
        let adapter = try communicator.createObjectAdapterWithRouter(name: "", rtr: router)
        try test(adapter.getPublishedEndpoints().count == 1)
        try test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 23456 -t 30000")
        do {
            try adapter.setPublishedEndpoints(router.ice_getEndpoints())
            try test(false)
        } catch let error as Ice.LocalException {
            try test(error.ice_id() == "std::invalid_argument")
            try test(
                error.message == "can't set published endpoints on object adapter associated with a router")
        }
        adapter.destroy()

        do {
            routerId.name = "test"
            router = uncheckedCast(prx: base.ice_identity(routerId), type: Ice.RouterPrx.self)
            _ = try communicator.createObjectAdapterWithRouter(name: "", rtr: router)
            try test(false)
        } catch is Ice.OperationNotExistException {
            // Expected: the "test" object doesn't implement Ice::Router!
        }

        do {
            router = try uncheckedCast(
                prx: communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 1))")!,
                type: Ice.RouterPrx.self)
            _ = try communicator.createObjectAdapterWithRouter(name: "", rtr: router)
            try test(false)
        } catch is Ice.ConnectFailedException {}
    }
    output.writeLine("ok")

    output.write("testing object adapter creation with port in use... ")
    do {
        let adapter1 = try communicator.createObjectAdapterWithEndpoints(
            name: "Adpt1",
            endpoints: helper.getTestEndpoint(num: 10))
        do {
            _ = try communicator.createObjectAdapterWithEndpoints(
                name: "Adpt2",
                endpoints: helper.getTestEndpoint(num: 10))
            try test(false)
        } catch is Ice.LocalException {
            // Expected can't re-use the same endpoint.
        }
        adapter1.destroy()
    }
    output.writeLine("ok")

    output.write("deactivating object adapter in the server... ")
    try await obj.deactivate()
    output.writeLine("ok")

    output.write("testing whether server is gone... ")
    if try await obj.ice_getConnection() == nil {  // collocated
        try await obj.ice_ping()
        output.writeLine("ok")
    } else {
        do {
            try await obj.ice_ping()
            try test(false)
        } catch is Ice.LocalException {
            output.writeLine("ok")
        }
    }
}
