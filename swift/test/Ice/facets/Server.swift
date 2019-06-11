//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {

        let communicator = try self.initialize(args)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))

        let adapter = try communicator.createObjectAdapter("TestAdapter")

        let d = DI()
        try adapter.add(servant: DDisp(d), id: Ice.stringToIdentity("d"))
        try adapter.addFacet(servant: DDisp(d), id: Ice.stringToIdentity("d"), facet: "facetABCD")

        let f = FI()
        try adapter.addFacet(servant: FDisp(f), id: Ice.stringToIdentity("d"), facet: "facetEF")

        let h = HI(communicator: communicator)
        try adapter.addFacet(servant: HDisp(h), id: Ice.stringToIdentity("d"), facet: "facetGH")

        try adapter.activate()

        serverReady()
        communicator.waitForShutdown()
    }
}
