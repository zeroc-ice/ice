//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

func onewaysAMI(_ helper: TestHelper, _ proxy: MyClassPrx) throws {
    let p = uncheckedCast(prx: proxy.ice_oneway(), type: MyClassPrx.self)

    do {
        let group = DispatchGroup()
        group.enter()
        _ = p.ice_pingAsync { _ in
            group.leave()
        }
        group.wait()
    }

    do {
        try firstly {
            p.ice_isAAsync(id: ice_staticId(MyClassPrx.self))
        }.done { _ in
            try helper.test(false)
        }.wait()
    } catch is Ice.TwowayOnlyException {}

    do {
        try firstly {
            p.ice_idsAsync()
        }.done { _ in
            try helper.test(false)
        }.wait()
    } catch is Ice.TwowayOnlyException {}

    do {
        let group = DispatchGroup()
        group.enter()
        _ = p.opVoidAsync { _ in
            group.leave()
        }
        group.wait()
    }

    do {
        let group = DispatchGroup()
        group.enter()
        _ = p.opIdempotentAsync { _ in
            group.leave()
        }
        group.wait()
    }

    do {
        let group = DispatchGroup()
        group.enter()
        _ = p.opNonmutatingAsync { _ in
            group.leave()
        }
        group.wait()
    }

    do {
        try firstly {
            p.opByteAsync(p1: 0xFF, p2: 0x0F)
        }.done { _ in
            try helper.test(false)
        }.wait()
    } catch is Ice.TwowayOnlyException {}
}
