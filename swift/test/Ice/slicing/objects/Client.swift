// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.AcceptClassCycles", value: "1")
        properties.setProperty(key: "Ice.SliceLoader.NotFoundCacheSize", value: "5")
        properties.setProperty(key: "Ice.Warn.SliceLoader", value: "0")  // comment out to see the warning

        var initData = InitializationData()
        initData.properties = properties
        initData.sliceLoader = CompositeSliceLoader(
            DefaultSliceLoader("IceSlicingObjects"),
            DefaultSliceLoader("IceSlicingObjectsClient")
        )

        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let testPrx = try await allTests(self)
        try await testPrx.shutdown()
    }
}
