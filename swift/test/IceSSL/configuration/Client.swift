//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class Client: TestHelperI {
    override public func run(args: [String]) throws {
        do {
            let communicator = try initialize(args)
            defer {
                communicator.destroy()
            }
            
            guard let resourcePath = Bundle.main.resourcePath else {
                fatalError("Bundle resources missing")
            }
            
//            print(Bundle(for: Self.self).resourcePath)
//            Bundle(for: )
//            print(Bundle.main.url(forResource: "cacert", withExtension: "pem"))
            
            let certsDir = resourcePath.appending("certs")
        
            
            
            try FileManager.default.contentsOfDirectory(atPath: certsDir).forEach { file in
                print(file)
            }

            let factory = try allTests(self, certsDir)
            try factory.shutdown()
        }
    }
}
