// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

func oneways(_ helper: TestHelper, _ prx: MyClassPrx) async throws {
    let p = prx.ice_oneway()

    try await p.ice_ping()
    try await p.opVoid()
    try await p.opIdempotent()

    // Calling a ["oneway"] operation on a oneway proxy succeeds.
    try await p.opOneway()

    // Calling a ["oneway"] operation on a twoway proxy throws OnewayOnlyException.
    do {
        try await prx.ice_twoway().opOneway()
        try helper.test(false)
    } catch is Ice.OnewayOnlyException {}

    do {
        _ = try await p.opByte(p1: 0xFF, p2: 0x0F)
        try helper.test(false)
    } catch is Ice.TwowayOnlyException {}
}
