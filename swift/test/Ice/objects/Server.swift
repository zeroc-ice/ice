//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = ["IceObjects"]
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        try communicator.getValueFactoryManager().add(factory: { _ in II() }, id: "::Test::I")
        try communicator.getValueFactoryManager().add(factory: { _ in JI() }, id: "::Test::J")
        try communicator.getValueFactoryManager().add(factory: { _ in HI() }, id: "::Test::H")

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: InitialDisp(InitialI(adapter)), id: Ice.stringToIdentity("initial"))
        try adapter.add(servant: F2Disp(F2I()), id: Ice.stringToIdentity("F21"))
        try adapter.add(servant: BlobjectDisp(UnexpectedObjectExceptionTestI()),
                        id: Ice.stringToIdentity("uoet"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
