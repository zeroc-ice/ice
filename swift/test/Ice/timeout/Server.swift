//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()

        let properties = try Ice.createProperties(args)

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")

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

        let communicator = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "ControllerAdapter.Endpoints", value: getTestEndpoint(num: 1))
        communicator.getProperties().setProperty(key: "ControllerAdapter.ThreadPool.Size", value: "1")

        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: TimeoutDisp(TimeoutI()), id: Ice.stringToIdentity("timeout"))
        try adapter.activate()

        let controllerAdapter = try communicator.createObjectAdapter("ControllerAdapter")
        try controllerAdapter.add(servant: ControllerDisp(ControllerI(adapter)), id: Ice.stringToIdentity("controller"))
        try controllerAdapter.activate()

        serverReady()
        communicator.waitForShutdown()
    }
}
