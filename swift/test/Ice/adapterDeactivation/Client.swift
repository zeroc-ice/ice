//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

class Client: TestHelperI {
    override func run(args: [String]) throws {
        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }
        try allTests(self)
    }
}
