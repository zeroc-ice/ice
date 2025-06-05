// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)

        //
        // The client sends large messages to cause the transport
        // buffers to fill up.
        //
        properties.setProperty(key: "Ice.MessageSizeMax", value: "20000")

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty(key: "Ice.TCP.RcvSize", value: "50000")

        let communicator = try initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(
            key: "ControllerAdapter.Endpoints", value: getTestEndpoint(num: 1))
        communicator.getProperties().setProperty(
            key: "ControllerAdapter.ThreadPool.Size", value: "1")

        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: TimeoutI(), id: Ice.stringToIdentity("timeout"))
        try adapter.activate()

        let controllerAdapter = try communicator.createObjectAdapter("ControllerAdapter")
        try controllerAdapter.add(
            servant: ControllerI(adapter), id: Ice.stringToIdentity("controller"))
        try controllerAdapter.activate()

        serverReady()
        communicator.waitForShutdown()
    }
}
