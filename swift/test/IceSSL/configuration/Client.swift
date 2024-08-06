// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

class Client: TestHelperI {
    override public func run(args: [String]) async throws {
        do {
            let communicator = try initialize(args)
            defer {
                communicator.destroy()
            }

            guard let resourcePath = Bundle.main.resourcePath else {
                fatalError("Bundle resources missing")
            }

            let certsDir = resourcePath.appending("/ice-test_IceSSL_configuration.bundle/certs")

            let factory = try await allTests(self, certsDir)
            try await factory.shutdown()
        }
    }
}
