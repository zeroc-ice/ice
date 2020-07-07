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

    deinit {
        PreservedI.counter -= 1
    }

    static var counter: Int32 = 0
}

class PNodeI: PNode {
    public required init() {
        PNodeI.counter += 1
        super.init()
    }

    deinit {
        PNodeI.counter -= 1
    }

    static var counter: Int32 = 0
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.AcceptClassCycles", value: "1")
        var initData = InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = ["IceSlicingObjects", "IceSlicingObjectsClient"]
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let testPrx = try allTests(self)
        try testPrx.shutdown()
    }
}
