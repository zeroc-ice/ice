//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit
import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let properties = try Ice.createProperties(args)
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.ACM.Timeout", value: "1")

        let communicator = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "TestAdapter.ACM.Timeout", value: "0")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: RemoteCommunicatorI(), id: Ice.stringToIdentity("communicator"))
        try adapter.activate()
        serverReady()
        communicator.getProperties().setProperty(key: "Ice.PrintAdapterReady", value: "0")
        communicator.waitForShutdown()
    }
}
