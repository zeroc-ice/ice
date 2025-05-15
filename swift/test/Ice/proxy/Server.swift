// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        //
        // We don't want connection warnings because of the timeout test.
        //
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let communicator = try initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(
            servant: MyDerivedClassDisp(MyDerivedClassI()),
            id: Ice.Identity(name: "test"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
