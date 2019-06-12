//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class TestI: TestIntf {
    var _helper: TestHelper

    init(_ helper: TestHelper) {
        _helper = helper
    }

    func baseAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw Base(b: "Base.b")
        }
    }

    func unknownDerivedAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw UnknownDerived(b: "UnknownDerived.b", ud: "UnknownDerived.ud")
        }
    }

    func knownDerivedAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownDerived(b: "KnownDerived.b", kd: "KnownDerived.kd")
        }
    }

    func knownDerivedAsKnownDerivedAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownDerived(b: "KnownDerived.b", kd: "KnownDerived.kd")
        }
    }

    func unknownIntermediateAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw UnknownIntermediate(b: "UnknownIntermediate.b", ui: "UnknownIntermediate.ui")
        }
    }

    func knownIntermediateAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownIntermediate(b: "KnownIntermediate.b", ki: "KnownIntermediate.ki")
        }
    }

    func knownMostDerivedAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownMostDerived(b: "KnownMostDerived.b", ki: "KnownMostDerived.ki", kmd: "KnownMostDerived.kmd")
        }
    }

    func knownIntermediateAsKnownIntermediateAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownIntermediate(b: "KnownIntermediate.b", ki: "KnownIntermediate.ki")
        }
    }

    func knownMostDerivedAsKnownIntermediateAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownMostDerived(b: "KnownMostDerived.b", ki: "KnownMostDerived.ki", kmd: "KnownMostDerived.kmd")
        }
    }

    func knownMostDerivedAsKnownMostDerivedAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownMostDerived(b: "KnownMostDerived.b",
                                   ki: "KnownMostDerived.ki",
                                   kmd: "KnownMostDerived.kmd")
        }
    }

    func unknownMostDerived1AsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw UnknownMostDerived1(b: "UnknownMostDerived1.b",
                                      ki: "UnknownMostDerived1.ki",
                                      umd1: "UnknownMostDerived1.umd1")
        }
    }

    func unknownMostDerived1AsKnownIntermediateAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw UnknownMostDerived1(b: "UnknownMostDerived1.b",
                                      ki: "UnknownMostDerived1.ki",
                                      umd1: "UnknownMostDerived1.umd1")
        }
    }

    func unknownMostDerived2AsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw UnknownMostDerived2(b: "UnknownMostDerived2.b",
                                      ui: "UnknownMostDerived2.ui",
                                      umd2: "UnknownMostDerived2.umd2")
        }
    }

    func unknownMostDerived2AsBaseCompactAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw UnknownMostDerived2(b: "UnknownMostDerived2.b",
                                      ui: "UnknownMostDerived2.ui",
                                      umd2: "UnknownMostDerived2.umd2")
        }
    }

    func knownPreservedAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownPreservedDerived(b: "base",
                                        kp: "preserved",
                                        kpd: "derived")
        }
    }

    func knownPreservedAsKnownPreservedAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            throw KnownPreservedDerived(b: "base",
                                        kp: "preserved",
                                        kpd: "derived")
        }
    }

    func relayKnownPreservedAsBaseAsync(r: RelayPrx?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let p = try uncheckedCast(prx: current.con!.createProxy(r!.ice_getIdentity()),
                                      type: RelayPrx.self)
            try p.knownPreservedAsBase()
            try _helper.test(false)
        }
    }

    func relayKnownPreservedAsKnownPreservedAsync(r: RelayPrx?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let p = try uncheckedCast(prx: current.con!.createProxy(r!.ice_getIdentity()),
                                      type: RelayPrx.self)
            try p.knownPreservedAsKnownPreserved()
            try _helper.test(false)
        }
    }

    func unknownPreservedAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let ex = SPreserved2()
            ex.b = "base"
            ex.kp = "preserved"
            ex.kpd = "derived"
            ex.p1 = SPreservedClass(bc: "bc", spc: "spc")
            ex.p2 = ex.p1
            throw ex
        }
    }

    func unknownPreservedAsKnownPreservedAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let ex = SPreserved2()
            ex.b = "base"
            ex.kp = "preserved"
            ex.kpd = "derived"
            ex.p1 = SPreservedClass(bc: "bc", spc: "spc")
            ex.p2 = ex.p1
            throw ex
        }
    }

    func relayUnknownPreservedAsBaseAsync(r: RelayPrx?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let p = try uncheckedCast(prx: current.con!.createProxy(r!.ice_getIdentity()),
                                      type: RelayPrx.self)
            try p.unknownPreservedAsBase()
            try _helper.test(false)
        }
    }

    func relayUnknownPreservedAsKnownPreservedAsync(r: RelayPrx?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let p = try uncheckedCast(prx: current.con!.createProxy(r!.ice_getIdentity()),
                                      type: RelayPrx.self)
            try p.unknownPreservedAsKnownPreserved()
            try _helper.test(false)
        }
    }

    func shutdownAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            current.adapter!.getCommunicator().shutdown()
            seal.fulfill(())
        }
    }
}
