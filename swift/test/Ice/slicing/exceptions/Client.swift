//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        initData.classResolverPrefix = ["IceSlicingExceptions", "IceSlicingExceptionsClient"]
        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }
        let testIntf = try allTests(self)
        try testIntf.shutdown()
    }
}
