//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Foundation

class I1: I {
    func opS(s1: S, current: Current) throws -> (returnValue: S, s2: S) {
        return (s1, s1)
    }

    func opSSeq(s1: SSeq, current: Current) throws -> (returnValue: SSeq, s2: SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: SMap, current: Current) throws -> (returnValue: SMap, s2: SMap) {
        return (s1, s1)
    }

    func opC(c1: C?, current: Current) throws -> (returnValue: C?, c2: C?) {
        return (c1, c1)
    }

    func opCSeq(s1: CSeq, current: Current) throws -> (returnValue: CSeq, s2: CSeq) {
        return (s1, s1)
    }

    func opCMap(c1: CMap, current: Current) throws -> (returnValue: CMap, c2: CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class I2: InnerI {
    func opS(s1: InnerInner2S, current: Current) throws -> (returnValue: InnerInner2S, s2: InnerInner2S) {
        return (s1, s1)
    }

    func opSSeq(s1: InnerInner2SSeq, current: Current) throws -> (returnValue: InnerInner2SSeq, s2: InnerInner2SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: InnerInner2SMap, current: Current) throws -> (returnValue: InnerInner2SMap, s2: InnerInner2SMap) {
        return (s1, s1)
    }

    func opC(c1: InnerInner2C?, current: Current) throws -> (returnValue: InnerInner2C?, c2: InnerInner2C?) {
        return (c1, c1)
    }

    func opCSeq(c1: InnerInner2CSeq, current: Current) throws -> (returnValue: InnerInner2CSeq, c2: InnerInner2CSeq) {
        return (c1, c1)
    }

    func opCMap(c1: InnerInner2CMap, current: Current) throws -> (returnValue: InnerInner2CMap, c2: InnerInner2CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class I3: InnerInner2I {
    func opS(s1: InnerInner2S, current: Current) throws -> (returnValue: InnerInner2S, s2: InnerInner2S) {
        return (s1, s1)
    }

    func opSSeq(s1: InnerInner2SSeq, current: Current) throws -> (returnValue: InnerInner2SSeq, s2: InnerInner2SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: InnerInner2SMap, current: Current) throws -> (returnValue: InnerInner2SMap, s2: InnerInner2SMap) {
        return (s1, s1)
    }

    func opC(c1: InnerInner2C?, current: Current) throws -> (returnValue: InnerInner2C?, c2: InnerInner2C?) {
        return (c1, c1)
    }

    func opCSeq(c1: InnerInner2CSeq, current: Current) throws -> (returnValue: InnerInner2CSeq, c2: InnerInner2CSeq) {
        return (c1, c1)
    }

    func opCMap(c1: InnerInner2CMap, current: Current) throws -> (returnValue: InnerInner2CMap, c2: InnerInner2CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class I4: InnerTestInner2I {
    func opS(s1: S, current: Current) throws -> (returnValue: S, s2: S) {
        return (s1, s1)
    }

    func opSSeq(s1: SSeq, current: Current) throws -> (returnValue: SSeq, s2: SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: SMap, current: Current) throws -> (returnValue: SMap, s2: SMap) {
        return (s1, s1)
    }

    func opC(c1: C?, current: Current) throws -> (returnValue: C?, c2: C?) {
        return (c1, c1)
    }

    func opCSeq(c1: CSeq, current: Current) throws -> (returnValue: CSeq, c2: CSeq) {
        return (c1, c1)
    }

    func opCMap(c1: CMap, current: Current) throws -> (returnValue: CMap, c2: CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
