//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()

        var restArgs = args
        let properties = try createTestProperties(&restArgs)
        properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        properties.setProperty(key: "Ice.UDP.RcvSize", value: "16384")

        let communicator = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        let num = restArgs.count == 1 ? Int32(restArgs[0]) : 1
        print("restArgs[0]: \(restArgs[0]) num: \(num ?? 0)")
        communicator.getProperties().setProperty(key: "ControlAdapter.Endpoints",
                                                 value: getTestEndpoint(num: num ?? 0, prot: "tcp"))
        let adapter = try communicator.createObjectAdapter("ControlAdapter")
        try adapter.add(servant: TestIntfDisp(TestIntfI()), id: Ice.stringToIdentity("control"))
        try adapter.activate()
        serverReady()
        if num == 0 {
            communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                     value: getTestEndpoint(num: 0, prot: "udp"))
            let adapter2 = try communicator.createObjectAdapter("TestAdapter")
            try adapter2.add(servant: TestIntfDisp(TestIntfI()), id: Ice.stringToIdentity("test"))
            try adapter2.activate()
        }

        var endpoint: String = ""
        //
        // Use loopback to prevent other machines to answer.
        //
        if properties.getProperty("Ice.IPv6") == "1" {
            endpoint += "udp -h \"ff15::1:1\" --interface \"::1\""
        } else {
            endpoint += "udp -h 239.255.1.1 --interface 127.0.0.1"
        }
        endpoint += " -p "
        endpoint += "\(getTestPort(properties: properties, num: 10))"
        communicator.getProperties().setProperty(key: "McastTestAdapter.Endpoints", value: endpoint)
        let mcastAdapter = try communicator.createObjectAdapter("McastTestAdapter")
        try mcastAdapter.add(servant: TestIntfDisp(TestIntfI()), id: Ice.stringToIdentity("test"))
        try mcastAdapter.activate()
        communicator.waitForShutdown()
    }
}
