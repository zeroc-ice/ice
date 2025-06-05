// Copyright (c) ZeroC, Inc.

import Dispatch
import Ice
import TestCommon

class Server: TestHelperI, @unchecked Sendable {
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
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
