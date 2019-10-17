//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Default.Locator",
                               value: "locator:\(getTestEndpoint(properties: properties, num: 0))")
        var initData = Ice.InitializationData()
        initData.properties = properties

        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        try allTests(self)
    }
}
