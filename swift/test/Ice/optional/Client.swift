// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        let customSliceLoader = CustomSliceLoader(helper: self)
        initData.sliceLoader = CompositeSliceLoader(
            customSliceLoader, DefaultSliceLoader("IceOptional"))
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let initial = try await allTests(self, customSliceLoader: customSliceLoader)
        try await initial.shutdown()
    }
}
