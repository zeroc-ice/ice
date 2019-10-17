//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Client: TestHelperI {
    public override func run(args: [String]) throws {
        do {
            var properties = try createTestProperties(args)
            properties.setProperty(key: "Ice.RetryIntervals", value: "0 1 10 1")

            //
            // This test kills connections, so we don't want warnings.
            //
            properties.setProperty(key: "Ice.Warn.Connections", value: "0")
            let communicator = try initialize(properties)
            defer {
                communicator.destroy()
            }

            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            properties = communicator.getProperties().clone()
            properties.setProperty(key: "Ice.RetryIntervals", value: "0 1 10000")
            let communicator2 = try self.initialize(properties)
            defer {
                communicator2.destroy()
            }

            let r = try allTests(helper: self,
                                 communicator2: communicator2,
                                 ref: "retry:\(self.getTestEndpoint(num: 0))")
            try r.shutdown()
        }
    }
}
