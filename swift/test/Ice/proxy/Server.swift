//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit
import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Server()
    }
}

class MyDerivedClassI: MyDerivedClass {
    var _ctx: [String: String]

    public init() {
        _ctx = [String: String]()
    }

    public func echo(obj: Ice.ObjectPrx?, current _: Ice.Current) throws -> Ice.ObjectPrx? {
        return obj
    }

    public func shutdown(current: Ice.Current) throws {
        guard let adapter = current.adapter else {
            precondition(false)
        }
        adapter.getCommunicator().shutdown()
    }

    public func getContext(current: Ice.Current) throws -> [String: String] {
        return _ctx
    }

    public func ice_isA(s: String, current: Ice.Current) throws -> Bool {
        _ctx = current.ctx
        return try ice_ids(current: current).contains(s)
    }
}

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()

        let (properties, _) = try Ice.createProperties(args: args)
        //
        // We don't want connection warnings because of the timeout test.
        //
        try properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        try properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let communicator = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        try communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        _ = try adapter.add(servant: MyDerivedClassI(), id: Ice.stringToIdentity("test"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
