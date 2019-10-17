//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        initData.classResolverPrefix = ["IceScope"]
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        try allTests(helper: self)
    }
}
