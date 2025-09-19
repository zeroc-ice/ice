// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(
            key: "Ice.Default.Locator",
            value: "locator:\(getTestEndpoint(properties: properties, num: 0))")
        let communicator = try initialize(Ice.InitializationData(properties: properties))
        defer {
            communicator.destroy()
        }
        try await allTests(self)
    }
}
