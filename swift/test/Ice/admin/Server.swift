//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                 value: "\(getTestEndpoint(num: 0)) -t 10000")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        let id = try Ice.stringToIdentity("factory")
        try adapter.add(servant: RemoteCommunicatorFactoryDisp(RemoteCommunicatorFactoryI()),
                        id: id)
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
