//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {

        let writer = getWriter()

        var initData = Ice.InitializationData()
        let (properties, _) = try createTestProperties(args: args)
        properties.setProperty(key: "Ice.ThreadPool.Client.Size", value: "2")
        properties.setProperty(key: "Ice.ThreadPool.Client.SizeWarn", value: "0")
        properties.setProperty(key: "Ice.BatchAutoFlushSize", value: "100")
        initData.properties = properties

        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }
        let cl = try allTests(helper: self)
        try cl.shutdown()
    }
}
