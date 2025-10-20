// Copyright (c) ZeroC, Inc.

import Dispatch
import Ice
import TestCommon

class Collocated: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
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

        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(
            key: "ControllerAdapter.Endpoints",
            value: getTestEndpoint(num: 1))
        communicator.getProperties().setProperty(
            key: "ControllerAdapter.ThreadPool.Size",
            value: "1")

        let adapter = try communicator.createObjectAdapter("TestAdapter")
        let adapter2 = try communicator.createObjectAdapter("ControllerAdapter")

        try adapter.add(
            servant: TestI(helper: self),
            id: Ice.Identity(name: "test"))
        try adapter.add(
            servant: TestII(),
            id: Ice.stringToIdentity("test2"))
        // try adapter.activate() // Don't activate OA to ensure collocation is used.
        try adapter2.add(
            servant: TestControllerI(adapter: adapter),
            id: Ice.stringToIdentity("testController"))
        // try adapter2.activate() // Don't activate OA to ensure collocation is used.
        try await allTests(self, collocated: true)
    }
}
