// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Server: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")
        let initData = Ice.InitializationData(properties: properties, sliceLoader: DefaultSliceLoader("IceObjects"))
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(servant: InitialI(adapter), id: Ice.stringToIdentity("initial"))
        try adapter.add(servant: F2I(), id: Ice.stringToIdentity("F21"))
        try adapter.add(
            servant: UnexpectedObjectExceptionTestDispatcher(),
            id: Ice.stringToIdentity("uoet"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
