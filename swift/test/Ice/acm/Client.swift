//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class Client: TestHelperI {
    override func run(args: [String]) throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")

        let communicator = try initialize(properties)
        defer {
            communicator.destroy()
        }
        try allTests(helper: self)
    }
}
