//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit

public class BI: B {
    override public func ice_preMarshal() {
        self.preMarshalInvoked = true
    }

    override public func ice_postUnmarshal() {
        self.postUnmarshalInvoked = true
    }
}

public class CI: C {
    override public func ice_preMarshal() {
        preMarshalInvoked = true
    }

    override public func ice_postUnmarshal() {
        postUnmarshalInvoked = true
    }
}

public class DI: D {
    override public func ice_preMarshal() {
        preMarshalInvoked = true
    }

    override public func ice_postUnmarshal() {
        postUnmarshalInvoked = true
    }
}

public class EI: E {
    public required init() {
        super.init(i: 1, s: "hello")
    }

    public func checkValues() -> Bool {
        return i == 1 && s == "hello"
    }
}

public class FI: F {
    public required init() {
        super.init()
    }

    public init(e: E) {
        super.init(e1: e, e2: e)
    }

    public func checkValues() -> Bool {
        return e1 !== nil && e1 === e2
    }
}

public class HI: H {}

public class II: Ice.InterfaceByValue {
    public required init() {
        super.init(id: "::Test::I")
    }
}

public class JI: Ice.InterfaceByValue {
    public required init() {
        super.init(id: "::Test::J")
    }
}

class InitialI: Initial {
    var _adapter: Ice.ObjectAdapter
    var _b1: B
    var _b2: B
    var _c: C
    var _d: D
    var _e: E
    var _f: F

    init(_ adapter: Ice.ObjectAdapter) {
        _adapter = adapter
        _b1 = BI()
        _b2 = BI()
        _c = CI()
        _d = DI()
        _e = EI()
        _f = FI(e: _e)

        _b1.theA = _b2 // Cyclic reference to another B
        _b1.theB = _b1 // Self reference.
        _b1.theC = nil // Null reference.

        _b2.theA = _b2 // Self reference, using base.
        _b2.theB = _b1 // Cyclic reference to another B
        _b2.theC = _c // Cyclic reference to a C.

        _c.theB = _b2 // Cyclic reference to a B.

        _d.theA = _b1 // Reference to a B.
        _d.theB = _b2 // Reference to a B.
        _d.theC = nil // Reference to a C.
    }

    func getAll(current: Ice.Current) throws -> (b1: B?, b2: B?, theC: C?, theD: D?) {
        return (_b1, _b2, _c, _d)
    }

    func getMB(current: Current) throws -> B? {
        return _b1
    }

    func getB1(current: Ice.Current) throws -> B? {
        return _b1
    }

    func getB2(current: Ice.Current) throws -> B? {
        return _b2
    }

    func getC(current: Ice.Current) throws -> C? {
        return _c
    }

    func getD(current: Ice.Current) throws -> D? {
        return _d
    }

    func getE(current: Ice.Current) throws -> E? {
        return _e
    }

    func getF(current: Ice.Current) throws -> F? {
        return _f
    }

    func getI(current: Ice.Current) throws -> Ice.Value? {
        return II()
    }

    func getJ(current: Ice.Current) throws -> Ice.Value? {
        return JI()
    }

    func getH(current: Ice.Current) throws -> Ice.Value? {
        return HI()
    }

    func getK(current: Ice.Current) throws -> K? {
        return K(value: L(data: "l"))
    }

    func opValue(v1: Ice.Value?, current: Ice.Current) throws -> (returnValue: Ice.Value?, v2: Ice.Value?) {
        return (v1, v1)
    }

    func opValueSeq(v1: [Ice.Value?], current: Ice.Current) throws -> (returnValue: [Ice.Value?], v2: [Ice.Value?]) {
        return (v1, v1)
    }

    func opValueMap(v1: [String: Ice.Value?],
                    current: Ice.Current) throws -> (returnValue: [String: Ice.Value?],
                                                              v2: [String: Ice.Value?]) {
        return (v1, v1)
    }

    func setRecursive(p: Recursive?, current: Ice.Current) {}

    func supportsClassGraphDepthMax(current: Ice.Current) throws -> Bool {
        return true
    }

    func getD1(d1: D1?, current: Ice.Current) throws -> D1? {
        return d1
    }

    func throwEDerived(current: Ice.Current) throws {
        throw EDerived(a1: A1(name: "a1"),
                       a2: A1(name: "a2"),
                       a3: A1(name: "a3"),
                       a4: A1(name: "a4"))
    }

    func setG(theG: G?, current: Ice.Current) throws {}

    func setI(theI: Ice.Value?, current: Ice.Current) throws {}

    func opBaseSeq(inSeq: [Base?], current: Ice.Current) throws -> (returnValue: [Base?], outSeq: [Base?]) {
        return (inSeq, inSeq)
    }

    func getCompact(current: Ice.Current) throws -> Compact? {
        return CompactExt()
    }

    func shutdown(current: Ice.Current) throws {
        _adapter.getCommunicator().shutdown()
    }

    func getInnerA(current: Ice.Current) throws -> InnerA? {
        return InnerA(theA: _b1)
    }

    func getInnerSubA(current: Ice.Current) throws -> InnerSubA? {
        return InnerSubA(theA: InnerA(theA: _b1))
    }

    func throwInnerEx(current: Ice.Current) throws {
        throw InnerEx(reason: "Inner::Ex")
    }

    func throwInnerSubEx(current: Ice.Current) throws {
        throw InnerSubEx(reason: "Inner::Sub::Ex")
    }

    func getAMDMBAsync(current: Ice.Current) -> Promise<B?> {
        return Promise<B?> {seal in
            seal.fulfill(_b1)
        }
    }

    func opM(v1: M?, current: Ice.Current) throws -> (returnValue: M?, v2: M?) {
        return (v1, v1)
    }
}

class UnexpectedObjectExceptionTestI: Ice.Blobject {
    func ice_invoke(inEncaps: [UInt8], current: Ice.Current) throws -> (ok: Bool, outParams: [UInt8]) {
        let communicator = current.adapter!.getCommunicator()
        let ostr = Ice.OutputStream(communicator: communicator)
        _ = ostr.startEncapsulation(encoding: current.encoding, format: .DefaultFormat)
        let ae = AlsoEmpty()
        ostr.write(ae)
        ostr.writePendingValues()
        ostr.endEncapsulation()
        return (true, ostr.finished())
    }
}
