// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class Client: TestHelperI {
    override public func run(args: [String]) async throws {
        do {
            let communicator = try initialize(args)
            defer {
                communicator.destroy()
            }
            let cl = try await allTests(self)
            try await cl.shutdown()
        }
    }
}
