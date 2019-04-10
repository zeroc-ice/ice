//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit
import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Collocated()
    }
}

class Collocated: TestHelperI {
    public override func run(args: [String]) throws {
        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }
        try communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        //
        // 2 threads are necessary to dispatch the collocated transient() call with AMI
        //
        let adapter = try communicator.createObjectAdapter(name: "TestAdapter",
                                                           queue: DispatchQueue(label: "Ice.adapterDeactivation.collocated",
                                                                                qos: .userInitiated,
                                                                                attributes: .concurrent))
        try adapter.addServantLocator(locator: ServantLocatorI(helper: self), category: "")
        try adapter.activate()
        try allTests(self)
        adapter.waitForDeactivate()
    }
}

