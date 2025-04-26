// Copyright (c) ZeroC, Inc.

import Ice
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

class CustomSliceLoader: SliceLoader {
    func newInstance(_ typeId: String) -> AnyObject? {
        switch typeId {
        case Preserved.ice_staticId():
            return PreservedI()
        case PNode.ice_staticId():
            return PNodeI()
        default:
            return nil
        }
    }
}

public class Client: TestHelperI {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.AcceptClassCycles", value: "1")
        var initData = InitializationData()
        initData.properties = properties
        initData.sliceLoader = CompositeSliceLoader(
            CustomSliceLoader(), DefaultSliceLoader("IceSlicingObjects", "IceSlicingObjectsClient"))

        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let testPrx = try await allTests(self)
        try await testPrx.shutdown()
    }
}
