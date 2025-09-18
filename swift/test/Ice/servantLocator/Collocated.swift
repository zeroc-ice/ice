// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Collocated: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let initData = try Ice.InitializationData(
            properties: createTestProperties(args), sliceLoader: DefaultSliceLoader("IceServantLocator"))
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.addServantLocator(
            locator: ServantLocatorI("category", self), category: "category")
        try adapter.addServantLocator(locator: ServantLocatorI("", self), category: "")
        try adapter.add(servant: TestI(), id: Ice.stringToIdentity("asm"))
        try adapter.add(
            servant: TestActivationI(self),
            id: Ice.stringToIdentity("test/activation"))
        // try adapter.activate() // Don't activate OA to ensure collocation is used.
        _ = try await allTests(self)
    }
}
