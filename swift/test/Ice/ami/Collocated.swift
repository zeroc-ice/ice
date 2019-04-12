//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Dispatch

public class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Collocated()
    }
}

class Collocated: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try createTestProperties(args: args)

        //
        // Disable collocation optimization to test async/await dispatch.
        //
        try properties.setProperty(key: "Ice.Default.CollocationOptimized", value: "0")

        //
        // This test kills connections, so we don't want warnings.
        //
        try properties.setProperty(key: "Ice.Warn.Connections", value: "0")

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        try properties.setProperty(key: "Ice.TCP.RcvSize", value: "50000")

        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }

        try communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                     value: getTestEndpoint(num: 0))
        try communicator.getProperties().setProperty(key: "ControllerAdapter.Endpoints",
                                                     value: getTestEndpoint(num: 1))
        try communicator.getProperties().setProperty(key: "ControllerAdapter.ThreadPool.Size",
                                                     value: "1")

        let adapter = try communicator.createObjectAdapter(name: "TestAdapter",
                                                           queue: DispatchQueue(label: "ice.ami.server.Server",
                                                                                qos: .userInitiated,
                                                                                attributes: .concurrent))
        let adapter2 = try communicator.createObjectAdapter(name: "ControllerAdapter",
                                                            queue: DispatchQueue(label: "ice.ami.server.Controller",
                                                                                 qos: .userInitiated))

        _ = try adapter.add(servant: TestI(helper: self), id: Ice.stringToIdentity("test"))
        _ = try adapter.add(servant: TestII(), id: Ice.stringToIdentity("test2"))
        try adapter.activate()
        _ = try adapter2.add(servant: TestControllerI(adapter: adapter), id: Ice.stringToIdentity("testController"))
        try adapter2.activate()
        try allTests(self, collocated: true)
    }
}
