//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {

        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        initData.classResolverPrefix = ["IceScope"]
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")

        _ = try adapter.add(servant: I1(), id: Ice.stringToIdentity("i1"))
        _ = try adapter.add(servant: I2(), id: Ice.stringToIdentity("i2"))
        _ = try adapter.add(servant: I3(), id: Ice.stringToIdentity("i3"))
        _ = try adapter.add(servant: I4(), id: Ice.stringToIdentity("i4"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
