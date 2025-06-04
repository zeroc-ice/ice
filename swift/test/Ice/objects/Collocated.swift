// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Collocated: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.AcceptClassCycles", value: "1")
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.sliceLoader = CompositeSliceLoader(
            CustomSliceLoader(), DefaultSliceLoader("IceObjects"))
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: InitialI(adapter), id: Ice.stringToIdentity("initial"))
        try adapter.add(servant: F2I(), id: Ice.stringToIdentity("F21"))
        try adapter.add(
            servant: UnexpectedObjectExceptionTestDispatcher(),
            id: Ice.stringToIdentity("uoet"))
        // try adapter.activate() // Don't activate OA to ensure collocation is used.

        let initial = try await allTests(self)
        // We must call shutdown even in the collocated case for cyclic dependency cleanup
        try await initial.shutdown()
    }
}
