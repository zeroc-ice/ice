//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Collocated: TestHelperI {
    public override func run(args: [String]) throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        initData.classResolverPrefix = ["IceServantLocator"]
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                 value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.addServantLocator(locator: ServantLocatorI("category", self), category: "category")
        try adapter.addServantLocator(locator: ServantLocatorI("", self), category: "")
        try adapter.add(servant: TestIntfDisp(TestI()), id: Ice.stringToIdentity("asm"))
        try adapter.add(servant: TestActivationDisp(TestActivationI(self)), id: Ice.stringToIdentity("test/activation"))
        //try adapter.activate() // Don't activate OA to ensure collocation is used.
        _ = try allTests(self)
    }
}
