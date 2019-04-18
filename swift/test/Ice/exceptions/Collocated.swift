//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Collocated()
    }
}

class DummyLogger: Ice.Logger {
    func print(_ message: String) {}

    func trace(category: String, message: String) {}

    func warning(_ message: String) {}

    func error(_ message: String) {}

    func getPrefix() -> String {
        return ""
    }

    func cloneWithPrefix(_ prefix: String) -> Logger {
        return DummyLogger()
    }
}

class Collocated: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try self.createTestProperties(args: args)
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.MessageSizeMax", value: "10") // 10KB max

        var initData = Ice.InitializationData()
        initData.properties = properties

        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "TestAdapter2.Endpoints", value: getTestEndpoint(num: 1))
        communicator.getProperties().setProperty(key: "TestAdapter2.MessageSizeMax", value: "0")
        communicator.getProperties().setProperty(key: "TestAdapter3.Endpoints", value: getTestEndpoint(num: 2))
        communicator.getProperties().setProperty(key: "TestAdapter3.MessageSizeMax", value: "1")

        let adapter = try communicator.createObjectAdapter("TestAdapter")

        let obj = ThrowerI()
        _ = try adapter.add(servant: obj, id: Ice.stringToIdentity("thrower"))

        try adapter.activate()

        _ = try allTests(self)
    }
}
