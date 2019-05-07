//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit
import Ice
import TestCommon

class ServerAMD: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()

        let properties = try Ice.createProperties(args)
        //
        // We don't want connection warnings because of the timeout test.
        //
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let communicator = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: MyDerivedClassI(), id: Ice.stringToIdentity("test"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
