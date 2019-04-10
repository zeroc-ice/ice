//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit
import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Server()
    }
}

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()

        let (properties, _) = try Ice.createProperties(args: args)
        try properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        try properties.setProperty(key: "Ice.ACM.Timeout", value: "1")

        let communicator = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        try communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        try communicator.getProperties().setProperty(key: "TestAdapter.ACM.Timeout", value: "0")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        _ = try adapter.add(servant: RemoteCommunicatorI(), id: Ice.stringToIdentity("communicator"))
        try adapter.activate()
        serverReady()
        try communicator.getProperties().setProperty(key: "Ice.PrintAdapterReady", value: "0")
        communicator.waitForShutdown()
    }
}
