//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let properties = try self.createTestProperties(args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = ["IceInvoke"]

        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }
        let cl = try allTests(self)
        try cl.shutdown()
    }
}
