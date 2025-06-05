// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        var initData = Ice.InitializationData()
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.ThreadPool.Client.Size", value: "2")
        properties.setProperty(key: "Ice.ThreadPool.Client.SizeWarn", value: "0")
        properties.setProperty(key: "Ice.BatchAutoFlushSize", value: "100")
        initData.properties = properties
        initData.sliceLoader = DefaultSliceLoader("IceOperations")
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let cl = try await allTests(helper: self)
        try await cl.shutdown()
    }
}
