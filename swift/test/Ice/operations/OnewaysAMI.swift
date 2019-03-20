//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class OnewaysAMI {
    public static func onewaysAMI(_ helper: TestHelper, _ proxy: MyClassPrx) throws {

        PromiseKit.conf.Q.map = .global()
        PromiseKit.conf.Q.return = .global()
        PromiseKit.conf.logHandler = { _ in }

        let p = uncheckedCast(prx: proxy.ice_oneway(), type: MyClassPrx.self)
        
        p?.ice_pingAsync()
    }

    public static func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        if !value {
            throw TestFailed.testFailed
        }
    }
}
