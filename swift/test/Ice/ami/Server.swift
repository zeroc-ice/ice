//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Dispatch

class Server: TestHelperI {
    public override func run(args: [String]) throws {

        let properties = try createTestProperties(args)

        //
        // Disable collocation optimization to test async/await dispatch.
        //
        properties.setProperty(key: "Ice.Default.CollocationOptimized", value: "0")

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty(key: "Ice.TCP.RcvSize", value: "50000")

        var initData = Ice.InitializationData()
        initData.properties = properties
        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                     value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "ControllerAdapter.Endpoints",
                                                     value: getTestEndpoint(num: 1))
        communicator.getProperties().setProperty(key: "ControllerAdapter.ThreadPool.Size",
                                                     value: "1")

        let adapter = try communicator.createObjectAdapter("TestAdapter")
        let adapter2 = try communicator.createObjectAdapter("ControllerAdapter")

        _ = try adapter.add(servant: TestI(helper: self), id: Ice.stringToIdentity("test"))
        _ = try adapter.add(servant: TestII(), id: Ice.stringToIdentity("test2"))
        try adapter.activate()
        _ = try adapter2.add(servant: TestControllerI(adapter: adapter), id: Ice.stringToIdentity("testController"))
        try adapter2.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
