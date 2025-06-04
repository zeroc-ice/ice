// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Collocated: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: "\(getTestEndpoint(num: 0)):\(getTestEndpoint(num: 0, prot: "udp"))"
        )
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: InitialI(adapter), id: Ice.stringToIdentity("initial"))
        // try adapter.activate() // Don't activate OA to ensure collocation is used.

        _ = try await allTests(self)
    }
}
