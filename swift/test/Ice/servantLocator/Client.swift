// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        initData.sliceLoader = DefaultSliceLoader("IceServantLocator")
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let obj = try await allTests(self)
        try await obj.shutdown()
    }
}
