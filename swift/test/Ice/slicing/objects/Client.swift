//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class PreservedI: Preserved {
    public required init() {
        PreservedI.counter += 1
        super.init()
    }

    static var counter: Int32 = 0
}

class PNodeI: PNode {
    public required init() {
        PNodeI.counter += 1
        super.init()
    }

    static var counter: Int32 = 0
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        var initData = InitializationData()
        initData.properties = try createTestProperties(args)
        initData.classResolverPrefix = ["IceSlicingObjects", "IceSlicingObjectsClient"]
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let testPrx = try allTests(self)
        try testPrx.shutdown()
    }
}
