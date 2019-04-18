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
        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.addServantLocator(locator: ServantLocatorI(helper: self), category: "")
        try adapter.activate()
        serverReady()
        adapter.waitForDeactivate()
    }
}
