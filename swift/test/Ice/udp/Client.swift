// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        do {
            var restArgs = args
            let properties = try createTestProperties(&restArgs)
            properties.setProperty(key: "Ice.Warn.Connections", value: "0")
            properties.setProperty(key: "Ice.UDP.SndSize", value: "16384")

            let communicator = try initialize(properties)
            defer {
                communicator.destroy()
            }
            try await allTests(self)

            let num = restArgs.count == 1 ? Int(restArgs[0]) : 1
            for i in 0..<(num ?? 1) {
                let prx = try communicator.stringToProxy(
                    "control:\(getTestEndpoint(num: Int32(i), prot: "tcp"))")!
                try await uncheckedCast(prx: prx, type: TestIntfPrx.self).shutdown()
            }
        }
    }
}
