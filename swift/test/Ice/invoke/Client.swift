// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.sliceLoader = DefaultSliceLoader("IceInvoke")

        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let cl = try await allTests(self)
        try await cl.shutdown()
    }
}
