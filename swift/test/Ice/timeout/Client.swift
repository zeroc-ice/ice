//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Client: TestHelperI {
    public override func run(args: [String]) throws {
        do {
            let communicator = try self.initialize(args)
            defer {
                communicator.destroy()
            }
            try allTests(helper: self)
        }
    }
}
