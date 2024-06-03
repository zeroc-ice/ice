//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Client: TestHelperI {
    override public func run(args _: [String]) throws {
        try allTests(self)
    }
}
