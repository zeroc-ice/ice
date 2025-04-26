// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI {
    override public func run(args: [String]) async throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        initData.sliceLoader = DefaultSliceLoader("IceScope")
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")

        try adapter.add(servant: MyInterfaceDisp(MyInterface1()), id: Ice.stringToIdentity("i1"))
        try adapter.add(servant: InnerMyInterfaceDisp(MyInterface2()), id: Ice.stringToIdentity("i2"))
        try adapter.add(servant: InnerInner2MyInterfaceDisp(MyInterface3()), id: Ice.stringToIdentity("i3"))
        try adapter.add(servant: InnerTestInner2MyInterfaceDisp(MyInterface4()), id: Ice.stringToIdentity("i4"))
        try adapter.add(servant: DifferentMyInterfaceDisp(DPMyInterface()), id: Ice.stringToIdentity("dpi"))
        try adapter.add(servant: NoPrefixMyInterfaceDisp(NPMyInterface()), id: Ice.stringToIdentity("npi"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
