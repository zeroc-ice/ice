//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Dispatch
import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(key: "TestAdapter1.Endpoints", value: getTestEndpoint(num: 0))
        communicator.getProperties().setProperty(key: "TestAdapter1.ThreadPool.Size", value: "5")
        communicator.getProperties().setProperty(key: "TestAdapter1.ThreadPool.SizeMax", value: "5")
        communicator.getProperties().setProperty(key: "TestAdapter1.ThreadPool.SizeWarn", value: "0")
        communicator.getProperties().setProperty(key: "TestAdapter1.ThreadPool.Serialize", value: "0")
        let adapter1 = try communicator.createObjectAdapter("TestAdapter1")
        try adapter1.add(servant: HoldDisp(HoldI(adapter: adapter1, helper: self)), id: Ice.stringToIdentity("hold"))

        communicator.getProperties().setProperty(key: "TestAdapter2.Endpoints", value: getTestEndpoint(num: 1))
        communicator.getProperties().setProperty(key: "TestAdapter2.ThreadPool.Size", value: "5")
        communicator.getProperties().setProperty(key: "TestAdapter2.ThreadPool.SizeMax", value: "5")
        communicator.getProperties().setProperty(key: "TestAdapter2.ThreadPool.SizeWarn", value: "0")
        communicator.getProperties().setProperty(key: "TestAdapter2.ThreadPool.Serialize", value: "1")
        let adapter2 = try communicator.createObjectAdapter("TestAdapter2")
        try adapter2.add(servant: HoldDisp(HoldI(adapter: adapter2, helper: self)), id: Ice.stringToIdentity("hold"))

        try adapter1.activate()
        try adapter2.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
