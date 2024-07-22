// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI {
    override public func run(args _: [String]) async throws {
        try await allTests(self)
    }
}
