//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Client: TestHelperI {
    public override func run(args: [String]) throws {
        do {
            let (communicator, _) = try self.initialize(args: args)
            defer {
                communicator.destroy()
            }
            let cl = try allTests(helper: self)
            try cl.shutdown()
        }
    }
}
