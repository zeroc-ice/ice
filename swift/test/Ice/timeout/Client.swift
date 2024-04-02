//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Client: TestHelperI {
    override public func run(args: [String]) throws {
        do {
            let communicator = try initialize(args)
            defer {
                communicator.destroy()
            }
            try allTests(helper: self)
        }
    }
}
