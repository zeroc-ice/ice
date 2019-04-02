//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

public class Client: TestHelperI {
    public override func run(args _: [String]) throws {
        let writer = getWriter()
        writer.write("testing default values... ")
        writer.writeLine("ok")
    }
}
