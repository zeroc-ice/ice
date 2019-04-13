//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Server()
    }
}

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                     value: "\(self.getTestEndpoint(num: 0)) -t 10000")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        let id = try Ice.stringToIdentity("factory")
        _ = try adapter.add(servant: RemoteCommunicatorFactoryI(), id: id)
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
