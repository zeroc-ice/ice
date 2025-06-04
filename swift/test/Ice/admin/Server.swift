// Copyright (c) ZeroC, Inc.

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
            value: "\(getTestEndpoint(num: 0)) -t 10000")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        let id = try Ice.stringToIdentity("factory")
        try adapter.add(
            servant: RemoteCommunicatorFactoryI(),
            id: id)
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
