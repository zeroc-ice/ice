//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class Collocated: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()

        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let communicator = try initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: RetryDisp(RetryI()), id: Ice.stringToIdentity("retry"))
        //try adapter.activate() // Don't activate OA to ensure collocation is used.
        _ = try allTests(helper: self)
    }
}
