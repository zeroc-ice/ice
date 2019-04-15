//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice

class TestI: TestIntf {
    func getAdapterName(current: Ice.Current) throws -> String {
        return current.adapter!.getName()
    }
}
