//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class Client: TestHelperI {
    public override func run(args: [String]) throws {
        do {
            let communicator = try initialize(args)
            defer {
                communicator.destroy()
            }
            var path = Bundle.main.bundlePath
            #if os(iOS) || os(watchOS) || os(tvOS)
                path += "/Frameworks/IceSSLConfiguration.bundle/certs"
            #else
                path += "/Contents/Frameworks/IceSSLConfiguration.bundle/Contents/Resources/certs"
            #endif

            let factory = try allTests(self, path)
            try factory.shutdown()
        }
    }
}
