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

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        do {
            let (communicator, _) = try self.initialize(args: args)
            defer {
                communicator.destroy()
            }
            let cl = try AllTests.allTests(helper: self)
            try cl.shutdown()
        }
    }
}
