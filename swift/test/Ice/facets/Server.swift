// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI {
    override public func run(args: [String]) async throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))

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
