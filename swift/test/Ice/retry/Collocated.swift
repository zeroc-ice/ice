// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Collocated: TestHelperI {
    override public func run(args: [String]) async throws {
        var properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.RetryIntervals", value: "0 1 10 1")

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")

        properties.setProperty(key: "TestAdapter.AdapterId", value: "RetryAdapter")

        let communicator = try initialize(properties)
        defer {
            communicator.destroy()
        }

        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        properties = communicator.getProperties().clone()
        properties.setProperty(key: "Ice.RetryIntervals", value: "0 1 10000")
        let communicator2 = try initialize(properties)
        defer {
            communicator2.destroy()
        }

        try communicator.createObjectAdapter("TestAdapter").add(
            servant: RetryI(),
            id: Ice.stringToIdentity("retry"))
        try communicator2.createObjectAdapter("TestAdapter").add(
            servant: RetryI(),
            id: Ice.stringToIdentity("retry"))

        // try adapter.activate() // Don't activate OA to ensure collocation is used.
        _ = try await allTests(helper: self, communicator2: communicator2, ref: "retry@RetryAdapter")
    }
}
