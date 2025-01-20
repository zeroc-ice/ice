// Copyright (c) ZeroC, Inc.

import Foundation
import TestCommon

public func getTestHelper(name: String) -> TestHelperI {
    guard let helperClass = Bundle.main.classNamed(name) as? TestHelperI.Type else {
        fatalError("test: `\(name)' not found")
    }

    return helperClass.init()
}
