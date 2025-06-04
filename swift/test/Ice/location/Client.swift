// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(
            key: "Ice.Default.Locator",
            value: "locator:\(getTestEndpoint(properties: properties, num: 0))")
        var initData = Ice.InitializationData()
        initData.properties = properties

        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        try await allTests(self)
    }
}
