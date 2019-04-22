//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return ServerAMD()
    }
}

class ServerAMD: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try createTestProperties(args: args)
        //
        // Its possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we supress this warning.
        //
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        //
        // We don't want connection warnings because of the timeout test.
        //
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")

        var initData = Ice.InitializationData()
        initData.properties = properties
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        _ = try adapter.add(servant: MyDerivedClassI(self), id: Ice.stringToIdentity("test"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
