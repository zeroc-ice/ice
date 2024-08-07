// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Collocated: TestHelperI {
    override public func run(args: [String]) async throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        //
        // 2 threads are necessary to dispatch the collocated transient() call with AMI
        //
        //TODO: this is likely no longer necessary
        communicator.getProperties().setProperty(key: "TestAdapter.ThreadPool.Size", value: "2")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.addServantLocator(locator: ServantLocatorI(helper: self), category: "")
        // try adapter.activate() // Don't activate OA to ensure collocation is used.
        try await allTests(self)
    }
}
