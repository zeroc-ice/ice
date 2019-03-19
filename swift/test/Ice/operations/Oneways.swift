//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Oneways {

    public static func oneways(_ helper: TestHelper, _ prx: MyClassPrx) throws {
        let p = prx.ice_oneway()

        try p.ice_ping()
        try p.opVoid()
        try p.opIdempotent()
        try p.opNonmutating()

        do {
            _ = try p.opByte(p1: 0xff, p2: 0x0f)
            try test(false)
        } catch is Ice.TwowayOnlyException {}
    }

    public static func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        if !value {
            throw TestFailed.testFailed
        }
    }
}
