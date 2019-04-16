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

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))

        let adapter = try communicator.createObjectAdapter("TestAdapter")

        let d = DI()
        _ = try adapter.add(servant: d, id: Ice.stringToIdentity("d"))
        _ = try adapter.addFacet(servant: d, id: Ice.stringToIdentity("d"), facet: "facetABCD")

        let f = FI()
        _ = try adapter.addFacet(servant: f, id: Ice.stringToIdentity("d"), facet: "facetEF")

        let h = HI(communicator: communicator)
        _ = try adapter.addFacet(servant: h, id: Ice.stringToIdentity("d"), facet: "facetGH")

        try adapter.activate()

        serverReady()
        communicator.waitForShutdown()
    }
}
