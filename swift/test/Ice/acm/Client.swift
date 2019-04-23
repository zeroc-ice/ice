//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit
import Ice
import TestCommon

class Client: TestHelperI {
     override func run(args: [String]) throws {

        let (properties, _) = try Ice.createProperties(args: args)
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")

        let communicator = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        try allTests(helper: self)
    }
}
