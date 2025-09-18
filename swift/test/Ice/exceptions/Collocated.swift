// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Collocated: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.MessageSizeMax", value: "10")  // 10KB max

        let initData = Ice.InitializationData(properties: properties, sliceLoader: DefaultSliceLoader("IceExceptions"))

        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(
            key: "TestAdapter2.Endpoints", value: getTestEndpoint(num: 1))
        communicator.getProperties().setProperty(key: "TestAdapter2.MessageSizeMax", value: "0")
        communicator.getProperties().setProperty(
            key: "TestAdapter3.Endpoints", value: getTestEndpoint(num: 2))
        communicator.getProperties().setProperty(key: "TestAdapter3.MessageSizeMax", value: "1")

        let adapter = try communicator.createObjectAdapter("TestAdapter")

        let obj = ThrowerI()
        try adapter.add(servant: obj, id: Ice.stringToIdentity("thrower"))

        // try adapter.activate() // Don't activate OA to ensure collocation is used.

        _ = try await allTests(self)
    }
}
