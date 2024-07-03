// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

class Server: TestHelperI {
    override public func run(args: [String]) throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }
        
        guard let resourcePath = Bundle.main.resourcePath else {
            fatalError("Bundle resources missing")
        }

        
        let certsDir = resourcePath.appending("/ice-test_IceSSL_configuration.bundle/certs")
        
        communicator.getProperties().setProperty(
            key: "TestAdapter.Endpoints",
            value: getTestEndpoint(num: 0, prot: "tcp"))
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.add(
            servant: SSLServerFactoryDisp(ServerFactoryI(defaultDir: certsDir, helper: self)),
            id: Ice.stringToIdentity("factory"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
