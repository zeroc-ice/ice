//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestCommon

public func runTest(name: String, args: [String]) throws{
    guard let helperClass =  Bundle.main.classNamed(name) as? TestHelperI.Type else {
        fatalError("test: `\(name)' not found")
    }

    let testHelper = helperClass.init()
    try testHelper.run(args: args)
}
