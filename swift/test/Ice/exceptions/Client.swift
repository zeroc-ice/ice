// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.MessageSizeMax", value: "10")  // 10KB max
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.sliceLoader = DefaultSliceLoader("IceExceptions")
        let communicator = try initialize(initData)
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        defer {
            communicator.destroy()
        }
        let thrower = try await allTests(self)
        try await thrower.shutdown()
    }
}
