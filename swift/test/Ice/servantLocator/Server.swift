// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI {
    override public func run(args: [String]) async throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        initData.sliceLoader = DefaultSliceLoader("IceServantLocator")
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.addServantLocator(locator: ServantLocatorI("category", self), category: "category")
        try adapter.addServantLocator(locator: ServantLocatorI("", self), category: "")
        try adapter.add(servant: TestIntfDisp(TestI()), id: Ice.stringToIdentity("asm"))
        try adapter.add(
            servant: TestActivationDisp(TestActivationI(self)),
            id: Ice.stringToIdentity("test/activation"))
        try adapter.activate()
        serverReady()
        adapter.waitForDeactivate()
    }
}
