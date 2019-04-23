//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try createTestProperties(args: args)
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = "IceObjects"
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        try communicator.getValueFactoryManager().add(factory: { _ in II() }, id: "::Test::I")
        try communicator.getValueFactoryManager().add(factory: { _ in JI() }, id: "::Test::J")
        try communicator.getValueFactoryManager().add(factory: { _ in HI() }, id: "::Test::H")

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        _ = try adapter.add(servant: InitialI(adapter), id: Ice.stringToIdentity("initial"))
        _ = try adapter.add(servant: UnexpectedObjectExceptionTestI(), id: Ice.stringToIdentity("uoet"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
