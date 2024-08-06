// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

func oneways(_ helper: TestHelper, _ prx: MyClassPrx) async throws {
    let p = prx.ice_oneway()

    try await p.ice_ping()
    try await p.opVoid()
    try await p.opIdempotent()

    do {
        _ = try await p.opByte(p1: 0xFF, p2: 0x0F)
        try helper.test(false)
    } catch is Ice.TwowayOnlyException {}
}
