//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try self.createTestProperties(args: args)
        properties.setProperty(key: "Ice.Default.Locator",
                               value: "locator:\(self.getTestEndpoint(properties: properties, num: 0))")
        var initData = Ice.InitializationData()
        initData.properties = properties

        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }
        try allTests(self)
    }
}
