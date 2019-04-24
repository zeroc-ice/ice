//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try createTestProperties(args: args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = "IceOptional"
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        try initial.shutdown()
    }
}
