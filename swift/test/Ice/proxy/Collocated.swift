// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Collocated: TestHelperI {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.ThreadPool.Client.Size", value: "2")
        properties.setProperty(key: "Ice.ThreadPool.Client.SizeWarn", value: "0")
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let communicator = try initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(
            servant: MyDerivedClassDisp(MyDerivedClassI()),
            id: Ice.Identity(name: "test"))
        // try adapter.activate() // Don't activate OA to ensure collocation is used.
        _ = try await allTests(self)
    }
}
