//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Client: TestHelperI {
    public override func run(args: [String]) throws {
        do {
            let (properties, _) = try createTestProperties(args: args)
            properties.setProperty(key: "Ice.RetryIntervals", value: "0 1 10 1");

            //
            // This test kills connections, so we don't want warnings.
            //
            properties.setProperty(key: "Ice.Warn.Connections", value: "0");
            let communicator = try self.initialize(properties)
            defer {
                communicator.destroy()
            }
            let r = try allTests(helper: self)
            try r.shutdown()
        }
    }
}
