//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class Client: TestHelperI {
    public override func run(args: [String]) throws {
        do {
            var restArgs = args
            let properties = try createTestProperties(&restArgs)
            properties.setProperty(key: "Ice.Warn.Connections", value: "0")
            properties.setProperty(key: "Ice.UDP.SndSize", value: "16384")

            let communicator = try initialize(properties)
            defer {
                communicator.destroy()
            }
            try allTests(self)

            let num = restArgs.count == 4 ? Int(restArgs[3]) : 1
            for i in 0 ..< (num ?? 1) {
                let prx = try communicator.stringToProxy("control:\(getTestEndpoint(num: Int32(i), prot: "tcp"))")!
                try uncheckedCast(prx: prx, type: TestIntfPrx.self).shutdown()
            }
        }
    }
}
