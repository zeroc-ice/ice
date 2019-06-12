//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func allTests(_ helper: TestHelper) throws {
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
    let obj = try checkedCast(prx: base, type: TestIntfPrx.self)!
    try test(obj == base)
    output.writeLine("ok")

    do {
        output.write("creating/destroying/recreating object adapter... ")
        var adapter = try communicator.createObjectAdapterWithEndpoints(name: "TransientTestAdapter",
                                                                        endpoints: "default")
        do {
            _ = try communicator.createObjectAdapterWithEndpoints(name: "TransientTestAdapter", endpoints: "default")
            try test(false)
        } catch is Ice.AlreadyRegisteredException {}
        adapter.destroy()

        //
        // Use a different port than the first adapter to avoid an "address already in use" error.
        //
        adapter = try communicator.createObjectAdapterWithEndpoints(name: "TransientTestAdapter", endpoints: "default")
        adapter.destroy()
        output.writeLine("ok")
    }

    output.write("creating/activating/deactivating object adapter in one operation... ")
    try obj.transient()
    try obj.transientAsync().wait()
    output.writeLine("ok")

    do {
        output.write("testing connection closure... ")
        for _ in 0 ..< 10 {
            var initData = Ice.InitializationData()
            initData.properties = communicator.getProperties().clone()
            let comm = try Ice.initialize(initData)
            _ = try comm.stringToProxy("test:\(helper.getTestEndpoint(num: 0))")!.ice_pingAsync()
            comm.destroy()
        }
        output.writeLine("ok")
    }

    output.write("testing object adapter published endpoints... ")
    do {
        communicator.getProperties().setProperty(key: "PAdapter.PublishedEndpoints",
                                                 value: "tcp -h localhost -p 12345 -t 30000")
        let adapter = try communicator.createObjectAdapter("PAdapter")
        try test(adapter.getPublishedEndpoints().count == 1)
        let endpt = adapter.getPublishedEndpoints()[0]
        try test(endpt.toString() == "tcp -h localhost -p 12345 -t 30000")
        let prx = try communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:" +
            "tcp -h localhost -p 12347 -t 10000")!
        try adapter.setPublishedEndpoints(prx.ice_getEndpoints())
        try test(adapter.getPublishedEndpoints().count == 2)
        var id = Ice.Identity()
        id.name = "dummy"

        try test(adapter.createProxy(id).ice_getEndpoints().elementsEqual(prx.ice_getEndpoints()) { $0 == $1 })
        try test(adapter.getPublishedEndpoints().elementsEqual(prx.ice_getEndpoints()) { $0 == $1 })
        try adapter.refreshPublishedEndpoints()
        try test(adapter.getPublishedEndpoints().count == 1)
        try test(adapter.getPublishedEndpoints()[0] == endpt)
        communicator.getProperties().setProperty(key: "PAdapter.PublishedEndpoints",
                                                 value: "tcp -h localhost -p 12345 -t 20000")
        try adapter.refreshPublishedEndpoints()
        try test(adapter.getPublishedEndpoints().count == 1)
        try test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 12345 -t 20000")
        adapter.destroy()
        try test(adapter.getPublishedEndpoints().count == 0)
    }
    output.writeLine("ok")

    if try obj.ice_getConnection() != nil {
        output.write("testing object adapter with bi-dir connection... ")
        let adapter = try communicator.createObjectAdapter("")
        try obj.ice_getConnection()!.setAdapter(adapter)
        try obj.ice_getConnection()!.setAdapter(nil)
        adapter.deactivate()
        do {
            try obj.ice_getConnection()!.setAdapter(adapter)
            try test(false)
        } catch is Ice.ObjectAdapterDeactivatedException {}
        output.writeLine("ok")
    }

    output.write("testing object adapter with router... ")
    do {
        var routerId = Ice.Identity()
        routerId.name = "router"
        var router = uncheckedCast(prx: base.ice_identity(routerId).ice_connectionId("rc"),
                                   type: Ice.RouterPrx.self)
        let adapter = try communicator.createObjectAdapterWithRouter(name: "", rtr: router)
        try test(adapter.getPublishedEndpoints().count == 1)
        try test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 23456 -t 30000")
        try adapter.refreshPublishedEndpoints()
        try test(adapter.getPublishedEndpoints().count == 1)
        try test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 23457 -t 30000")
        do {
            try adapter.setPublishedEndpoints(router.ice_getEndpoints())
            try test(false)
        } catch is Ice.RuntimeError {}
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
            router = try uncheckedCast(prx: communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 1))")!,
                                       type: Ice.RouterPrx.self)
            _ = try communicator.createObjectAdapterWithRouter(name: "", rtr: router)
            try test(false)
        } catch is Ice.ConnectFailedException {}
    }
    output.writeLine("ok")

    output.write("testing object adapter creation with port in use... ")
    do {
        let adapter1 = try communicator.createObjectAdapterWithEndpoints(name: "Adpt1",
                                                                         endpoints: helper.getTestEndpoint(num: 10))
        do {
            _ = try communicator.createObjectAdapterWithEndpoints(name: "Adpt2",
                                                                  endpoints: helper.getTestEndpoint(num: 10))
            try test(false)
        } catch is Ice.LocalException {
            // Expected can't re-use the same endpoint.
        }
        adapter1.destroy()
    }
    output.writeLine("ok")

    output.write("deactivating object adapter in the server... ")
    try obj.deactivate()
    output.writeLine("ok")

    output.write("testing whether server is gone... ")
    do {
        try obj.ice_ping()
        try test(false)
    } catch is Ice.LocalException {
        output.writeLine("ok")
    }
}
