// Copyright (c) ZeroC, Inc.

import Dispatch
import Ice
import TestCommon

class Server: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {

        let properties = try createTestProperties(args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.sliceLoader = DefaultSliceLoader("IceInvoke")
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: "\(getTestEndpoint(num: 0))")
        let adapter = try communicator.createObjectAdapter("TestAdapter")

        try adapter.addServantLocator(locator: ServantLocatorI(), category: "")
        try adapter.activate()

        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
