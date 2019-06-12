//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.MessageSizeMax", value: "10") // 10KB max
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = ["IceExceptions"]
        let communicator = try initialize(initData)
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        defer {
            communicator.destroy()
        }
        let thrower = try allTests(self)
        try thrower.shutdown()
    }
}
