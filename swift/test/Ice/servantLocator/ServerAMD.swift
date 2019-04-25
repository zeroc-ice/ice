//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class ServerAMD: TestHelperI {
    public override func run(args: [String]) throws {
        let (properties, _) = try createTestProperties(args: args)
        var initData = Ice.InitializationData()
        initData.properties = properties
        initData.classResolverPrefix = "IceServantLocator"
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                 value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "Ice.Warn.Dispatch", value: "0")
        
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        _ = try adapter.addServantLocator(locator: ServantLocatorI("category", self), category: "category")
        _ = try adapter.addServantLocator(locator: ServantLocatorI("", self), category: "")
        _ = try adapter.add(servant: TestI(), id: Ice.stringToIdentity("asm"))
        _ = try adapter.add(servant: TestActivationI(self), id: Ice.stringToIdentity("test/activation"))
        try adapter.activate()
        serverReady()
        adapter.waitForDeactivate()
    }
}
