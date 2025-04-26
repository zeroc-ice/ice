// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.AcceptClassCycles", value: "1")
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        var initData = InitializationData()
        initData.properties = properties
        initData.sliceLoader = CompositeSliceLoader(
            DefaultSliceLoader("IceSlicingObjects"),
            DefaultSliceLoader("IceSlicingObjectsServer")
        )
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: "\(getTestEndpoint(num: 0)) -t 2000")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: TestIntfDisp(TestI(self)), id: Ice.stringToIdentity("Test"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
