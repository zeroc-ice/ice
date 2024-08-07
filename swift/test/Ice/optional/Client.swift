// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = ["IceOptional"]
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let initial = try await allTests(self)
        try await initial.shutdown()
    }
}
