//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Client: TestHelperI {
    override public func run(args: [String]) throws {
        do {
            let properties = try createTestProperties(args)
            properties.setProperty(key: "Ice.Connection.ConnectTimeout", value: "1")
            properties.setProperty(key: "Ice.Connection.CloseTimeout", value: "1")
            let communicator = try initialize(properties)
            defer {
                communicator.destroy()
            }
            try allTests(helper: self)
        }
    }
}
