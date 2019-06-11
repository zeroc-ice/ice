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

        try adapter.add(servant: IDisp(I1()), id: Ice.stringToIdentity("i1"))
        try adapter.add(servant: InnerIDisp(I2()), id: Ice.stringToIdentity("i2"))
        try adapter.add(servant: InnerInner2IDisp(I3()), id: Ice.stringToIdentity("i3"))
        try adapter.add(servant: InnerTestInner2IDisp(I4()), id: Ice.stringToIdentity("i4"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
