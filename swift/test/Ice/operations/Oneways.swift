//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func oneways(_ helper: TestHelper, _ prx: MyClassPrx) throws {
    let p = prx.ice_oneway()

    try p.ice_ping()
    try p.opVoid()
    try p.opIdempotent()
    try p.opNonmutating()

    do {
        _ = try p.opByte(p1: 0xFF, p2: 0x0F)
        try helper.test(false)
    } catch is Ice.TwowayOnlyException {}
}
