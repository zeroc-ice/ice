//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class OnewaysAMI {
    public static func onewaysAMI(_ helper: TestHelper, _ proxy: MyClassPrx) throws {

        let p = uncheckedCast(prx: proxy.ice_oneway(), type: MyClassPrx.self)!

        do {
            let group = DispatchGroup()
            group.enter()
            _ = p.ice_pingAsync(sent: { _ in
                group.leave()
            })
            group.wait()
        }

        do {
            try firstly {
                p.ice_isAAsync(id: ice_staticId(MyClassPrx.self))
            }.done { _ in
                try test(false)
            }.wait()
        } catch is Ice.TwowayOnlyException {}

        do {
            try firstly {
                p.ice_idsAsync()
            }.done { _ in
                try test(false)
            }.wait()
        } catch is Ice.TwowayOnlyException {}

        do {
            let group = DispatchGroup()
            group.enter()
            _ = p.opVoidAsync(sent: { _ in
                group.leave()
            })
            group.wait()
        }

        do {
            let group = DispatchGroup()
            group.enter()
            _ = p.opIdempotentAsync(sent: { _ in
                group.leave()
            })
            group.wait()
        }

        do {
            let group = DispatchGroup()
            group.enter()
            _ = p.opNonmutatingAsync(sent: { _ in
                group.leave()
            })
            group.wait()
        }

        do {
            try firstly {
                p.opByteAsync(p1: 0xff, p2: 0x0f)
            }.done { _ in
                try test(false)
            }.wait()
        } catch is Ice.TwowayOnlyException {}
    }

    public static func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        if !value {
            throw TestFailed.testFailed
        }
    }
}
