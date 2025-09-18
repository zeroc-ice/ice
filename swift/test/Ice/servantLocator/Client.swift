// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let initData = try Ice.InitializationData(
            properties: createTestProperties(args), sliceLoader: DefaultSliceLoader("IceServantLocator"))
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let obj = try await allTests(self)
        try await obj.shutdown()
    }
}
