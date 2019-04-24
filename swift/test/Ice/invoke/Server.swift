//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Dispatch

class ServantLocatorI: Ice.ServantLocator {
    var _blobject: Ice.Object

    init (_ async: Bool) {
        if async {
            _blobject = BlobjectAsyncI()
        } else {
            _blobject = BlobjectI()
        }
    }

    func locate(_ curr: Ice.Current) -> (returnValue: Ice.Object?, cookie: AnyObject?) {
        return (_blobject, nil)
    }

    func finished(curr: Ice.Current, servant: Ice.Object, cookie: AnyObject?) {}

    func deactivate(_ category: String) {}
}

class Server: TestHelperI {
    public override func run(args: [String]) throws {

        let async = args.contains("--async")

        let properties  = try self.createTestProperties(args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = "IceInvoke"
        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: "\(getTestEndpoint(num: 0))")
        let adapter = try communicator.createObjectAdapter("TestAdapter")

        try adapter.addServantLocator(locator: ServantLocatorI(async), category: "")
        try adapter.activate()

        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
