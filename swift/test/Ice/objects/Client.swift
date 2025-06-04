// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.AcceptClassCycles", value: "1")
        var initData = InitializationData()
        initData.properties = properties
        initData.sliceLoader = CompositeSliceLoader(
            CustomSliceLoader(), DefaultSliceLoader("IceObjects"))
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let initial = try await allTests(self)
        try await initial.shutdown()
    }
}
