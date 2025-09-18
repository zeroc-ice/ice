// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.AcceptClassCycles", value: "1")
        let initData = Ice.InitializationData(properties: properties, sliceLoader: DefaultSliceLoader("IceOptional"))
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: InitialI(), id: Ice.stringToIdentity("initial"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
