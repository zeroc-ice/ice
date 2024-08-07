// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI {
    override public func run(args: [String]) async throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }
        let initial = try await allTests(self)
        try await initial.shutdown()
    }
}
