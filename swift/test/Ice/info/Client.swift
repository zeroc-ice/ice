//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let communicator = try self.initialize(args)
        defer {
            communicator.destroy()
        }
        try allTests(self)
    }
}
