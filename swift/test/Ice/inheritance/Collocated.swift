//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Collocated: TestHelperI {
    public override func run(args: [String]) throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: "\(getTestEndpoint(num: 0)):\(getTestEndpoint(num: 0, prot: "udp"))"
        )
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: InitialDisp(InitialI(adapter)), id: Ice.stringToIdentity("initial"))
        try adapter.activate()

        _ = try allTests(self)
    }
}
