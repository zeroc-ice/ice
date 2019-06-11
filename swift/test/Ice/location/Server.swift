//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Dispatch

class Server: TestHelperI {
    public override func run(args: [String]) throws {

        //
        // Register the server manager. The server manager creates a new
        // 'server'(a server isn't a different process, it's just a new
        // communicator and object adapter).
        //
        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.ThreadPool.Server.Size", value: "2")
        var initData = Ice.InitializationData()
        initData.properties = properties

        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "ServerManagerAdapter.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("ServerManagerAdapter")

        //
        // We also register a sample server locator which implements the
        // locator interface, this locator is used by the clients and the
        // 'servers' created with the server manager interface.
        //
        let registry = ServerLocatorRegistry()
        let object = ServerManagerI(registry: registry, helper: self)
        try adapter.add(servant: ServerManagerDisp(object), id: Ice.stringToIdentity("ServerManager"))
        try registry.addObject(adapter.createProxy(Ice.stringToIdentity("ServerManager")))
        let registryPrx = try uncheckedCast(prx: adapter.add(servant: TestLocatorRegistryDisp(registry),
                                                             id: Ice.stringToIdentity("registry")),
                                            type: Ice.LocatorRegistryPrx.self)

        let locator = ServerLocator(registry: registry, registryPrx: registryPrx)
        try adapter.add(servant: TestLocatorDisp(locator), id: Ice.stringToIdentity("locator"))

        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
