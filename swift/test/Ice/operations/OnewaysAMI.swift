// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

func onewaysAMI(_ helper: TestHelper, _ proxy: MyClassPrx) async throws {
    let p = uncheckedCast(prx: proxy.ice_oneway(), type: MyClassPrx.self)

    try await withCheckedThrowingContinuation { continuation in
        Task {
            try! await p.opVoidAsync { _ in
                continuation.resume(returning: ())
            }
        }
    }

    do {
        let _ = try await p.ice_isAAsync(id: ice_staticId(MyClassPrx.self))
        try helper.test(false)
    } catch is Ice.TwowayOnlyException {}

    do {
        let _ = try await p.ice_idsAsync()
        try helper.test(false)
    } catch is Ice.TwowayOnlyException {}

    try await withCheckedThrowingContinuation { continuation in
        Task {
            try! await p.opVoidAsync { _ in
                continuation.resume(returning: ())
            }
        }
    }

    try await withCheckedThrowingContinuation { continuation in
        Task {
            try! await p.opIdempotentAsync { _ in
                continuation.resume(returning: ())
            }
        }

    }

    do {
        let _ = try await p.opByteAsync(p1: 0xFF, p2: 0x0F)
        try helper.test(false)
    } catch is Ice.TwowayOnlyException {}

}
