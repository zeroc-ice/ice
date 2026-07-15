// Copyright (c) ZeroC, Inc.

import Dispatch
import Ice
import TestCommon

class Server: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        // Accept SSL clients that don't present a certificate.
        properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
        var initData = Ice.InitializationData()
        initData.properties = properties
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: "\(getTestEndpoint(num: 0)):\(getTestEndpoint(num: 0, prot: "udp"))"
        )
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: TestI(), id: Ice.Identity(name: "test"))
        try adapter.activate()
        serverReady()
        await communicator.shutdownCompleted()
    }
}
