// Copyright (c) ZeroC, Inc.

import Foundation
import Ice

public class BI: B {
    override public func ice_preMarshal() {
        preMarshalInvoked = true
    }

    override public func ice_postUnmarshal() {
        postUnmarshalInvoked = true
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

        _b1.theA = _b2  // Cyclic reference to another B
        _b1.theB = _b1  // Self reference.
        _b1.theC = nil  // Null reference.

        _b2.theA = _b2  // Self reference, using base.
        _b2.theB = _b1  // Cyclic reference to another B
        _b2.theC = _c  // Cyclic reference to a C.

        _c.theB = _b2  // Cyclic reference to a B.

        _d.theA = _b1  // Reference to a B.
        _d.theB = _b2  // Reference to a B.
        _d.theC = nil  // Reference to a C.
    }

    func getAll(current _: Ice.Current) async throws -> (b1: B?, b2: B?, theC: C?, theD: D?) {
        return (_b1, _b2, _c, _d)
    }

    func getMB(current _: Current) async throws -> B? {
        return _b1
    }

    func getB1(current _: Ice.Current) async throws -> B? {
        return _b1
    }

    func getB2(current _: Ice.Current) async throws -> B? {
        return _b2
    }

    func getC(current _: Ice.Current) async throws -> C? {
        return _c
    }

    func getD(current _: Ice.Current) async throws -> D? {
        return _d
    }

    func getE(current _: Ice.Current) async throws -> E? {
        return _e
    }

    func getF(current _: Ice.Current) async throws -> F? {
        return _f
    }

    func getK(current _: Ice.Current) async throws -> K? {
        return K(value: L(data: "l"))
    }

    func opValue(v1: Ice.Value?, current _: Ice.Current) async throws -> (
        returnValue: Ice.Value?, v2: Ice.Value?
    ) {
        return (v1, v1)
    }

    func opValueSeq(v1: [Ice.Value?], current _: Ice.Current) async throws -> (
        returnValue: [Ice.Value?], v2: [Ice.Value?]
    ) {
        return (v1, v1)
    }

    func opValueMap(
        v1: [String: Ice.Value?],
        current _: Ice.Current
    ) throws -> (
        returnValue: [String: Ice.Value?],
        v2: [String: Ice.Value?]
    ) {
        return (v1, v1)
    }

    func setRecursive(p _: Recursive?, current _: Ice.Current) {}

    func setCycle(r: Recursive?, current _: Ice.Current) {
        precondition(r != nil)
        precondition(r!.v === r)
        // break the cycle
        r!.v = nil
    }

    func acceptsClassCycles(current: Ice.Current) async throws -> Bool {
        let properties = current.adapter.getCommunicator().getProperties()
        return try properties.getIcePropertyAsInt("Ice.AcceptClassCycles") > 0
    }

    func getD1(d1: D1?, current _: Ice.Current) async throws -> D1? {
        return d1
    }

    func throwEDerived(current _: Ice.Current) async throws {
        throw EDerived(
            a1: A1(name: "a1"),
            a2: A1(name: "a2"),
            a3: A1(name: "a3"),
            a4: A1(name: "a4"))
    }

    func setG(theG _: G?, current _: Ice.Current) async throws {}

    func opBaseSeq(inSeq: [Base?], current _: Ice.Current) async throws -> (
        returnValue: [Base?], outSeq: [Base?]
    ) {
        return (inSeq, inSeq)
    }

    func getCompact(current _: Ice.Current) async throws -> Compact? {
        return CompactExt()
    }

    func shutdown(current _: Ice.Current) async throws {
        _b1.theA = nil  // Break cyclic reference.
        _b1.theB = nil  // Break cyclic reference.

        _b2.theA = nil  // Break cyclic reference.
        _b2.theB = nil  // Break cyclic reference.
        _b2.theC = nil  // Break cyclic reference.

        _c.theB = nil  // Break cyclic reference.

        _d.theA = nil  // Break cyclic reference.
        _d.theB = nil  // Break cyclic reference.
        _d.theC = nil  // Break cyclic reference.

        _adapter.getCommunicator().shutdown()
    }

    func getInnerA(current _: Ice.Current) async throws -> InnerA? {
        return InnerA(theA: _b1)
    }

    func getInnerSubA(current _: Ice.Current) async throws -> InnerSubA? {
        return InnerSubA(theA: InnerA(theA: _b1))
    }

    func throwInnerEx(current _: Ice.Current) async throws {
        throw InnerEx(reason: "Inner::Ex")
    }

    func throwInnerSubEx(current _: Ice.Current) async throws {
        throw InnerSubEx(reason: "Inner::Sub::Ex")
    }

    func getAMDMB(current _: Ice.Current) -> B? {
        return _b1
    }

    func opM(v1: M?, current _: Ice.Current) async throws -> (returnValue: M?, v2: M?) {
        return (v1, v1)
    }

    func opF1(f11: F1?, current _: Ice.Current) async throws -> (returnValue: F1?, f12: F1?) {
        return (f11, F1(name: "F12"))
    }

    func opF2(f21: F2Prx?, current: Current) async throws -> (returnValue: F2Prx?, f22: F2Prx?) {
        let prx = try current.adapter.getCommunicator().stringToProxy("F22")!
        return (f21, uncheckedCast(prx: prx, type: F2Prx.self))
    }

    func opF3(f31: F3?, current: Current) async throws -> (returnValue: F3?, f32: F3?) {
        let prx = try current.adapter.getCommunicator().stringToProxy("F22")!
        return (f31, F3(f1: F1(name: "F12"), f2: uncheckedCast(prx: prx, type: F2Prx.self)))
    }

    func hasF3(current _: Current) async throws -> Bool {
        return true
    }
}

class F2I: F2 {
    func op(current _: Current) async throws {}
}

class UnexpectedObjectExceptionTestDispatcher: Ice.Dispatcher {
    public func dispatch(_ request: IncomingRequest) async throws -> OutgoingResponse {
        let ae = AlsoEmpty()
        return request.current.makeOutgoingResponse(
            ae, formatType: nil
        ) { ostr, ae in
            ostr.write(ae)
            ostr.writePendingValues()
        }
    }
}
