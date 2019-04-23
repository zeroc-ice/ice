//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try createTestProperties(args: args)
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.MessageSizeMax", value: "10") // 10KB max
        var initData = Ice.InitializationData()
        initData.properties = properties
        let communicator = try self.initialize(initData)
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: self.getTestEndpoint(num: 0))
        defer {
            communicator.destroy()
        }
        let g = try allTests(self)
        try g.shutdown()
    }
}
