// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        //
        // Its possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        //
        // We don't want connection warnings because of the timeout test.
        //
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")

        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.sliceLoader = DefaultSliceLoader("IceOperations")
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(
            servant: MyDerivedClassI(self), id: Ice.Identity(name: "test"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
