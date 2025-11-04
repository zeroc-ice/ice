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

public class CustomSliceLoader: SliceLoader {
    public func newInstance(_ typeId: String) -> AnyObject? {
        switch typeId {
        case "::Test::B":
            return BI()
        case "::Test::C":
            return CI()
        case "::Test::D":
            return DI()
        default:
            return nil
        }
    }
}

final class InitialI: Initial, @unchecked Sendable {
    private let _adapter: Ice.ObjectAdapter
    private let _b1: B
    private let _b2: B
    private let _c: C
    private let _d: D

    init(_ adapter: Ice.ObjectAdapter) {
        _adapter = adapter
        _b1 = BI()
        _b2 = BI()
        _c = CI()
        _d = DI()

        _b1.theA = _b2  // Cyclic reference to another B
        _b1.theB = _b1  // Self reference.
        _b1.theC = nil  // nil reference.

        _b2.theA = _b2  // Self reference, using base.
        _b2.theB = _b1  // Cyclic reference to another B
        _b2.theC = _c  // Cyclic reference to a C.

        _c.theB = _b2  // Cyclic reference to a B.

        _d.theA = _b1  // Reference to a B.
        _d.theB = _b2  // Reference to a B.
        _d.theC = nil  // Reference to a C.
    }

    func getAll(current _: Ice.Current) -> (b1: B?, b2: B?, theC: C?, theD: D?) {
        return (_b1, _b2, _c, _d)
    }

    func opRenamedClass(r: SwiftClass?, current _: Ice.Current) -> SwiftClass? {
        return r
    }

    func getMB(current _: Current) -> B? {
        return _b1
    }

    func getB1(current _: Ice.Current) -> B? {
        return _b1
    }

    func getB2(current _: Ice.Current) -> B? {
        return _b2
    }

    func getC(current _: Ice.Current) -> C? {
        return _c
    }

    func getD(current _: Ice.Current) -> D? {
        return _d
    }

    func getK(current _: Ice.Current) -> K? {
        return K(value: L(data: "l"))
    }

    func opValue(v1: Ice.Value?, current _: Ice.Current) -> (
        returnValue: Ice.Value?, v2: Ice.Value?
    ) {
        return (v1, v1)
    }

    func opValueSeq(v1: [Ice.Value?], current _: Ice.Current) -> (
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

    func acceptsClassCycles(current: Ice.Current) throws -> Bool {
        let properties = current.adapter.getCommunicator().getProperties()
        return try properties.getIcePropertyAsInt("Ice.AcceptClassCycles") > 0
    }

    func getD1(d1: D1?, current _: Ice.Current) throws -> D1? {
        return d1
    }

    func throwEDerived(current _: Ice.Current) throws {
        throw EDerived(
            a1: A1(name: "a1"),
            a2: A1(name: "a2"),
            a3: A1(name: "a3"),
            a4: A1(name: "a4"))
    }

    func setG(theG _: G?, current _: Ice.Current) {}

    func opBaseSeq(inSeq: [Base?], current _: Ice.Current) -> (
        returnValue: [Base?], outSeq: [Base?]
    ) {
        return (inSeq, inSeq)
    }

    func getCompact(current _: Ice.Current) -> Compact? {
        return CompactExt()
    }

    func shutdown(current _: Ice.Current) {
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

    func getInnerA(current _: Ice.Current) -> InnerA? {
        return InnerA(theA: _b1)
    }

    func getInnerSubA(current _: Ice.Current) -> InnerSubA? {
        return InnerSubA(theA: InnerA(theA: _b1))
    }

    func throwInnerEx(current _: Ice.Current) throws {
        throw InnerEx(reason: "Inner::Ex")
    }

    func throwInnerSubEx(current _: Ice.Current) throws {
        throw InnerSubEx(reason: "Inner::Sub::Ex")
    }

    func getAMDMB(current _: Ice.Current) -> B? {
        return _b1
    }

    func opM(v1: M?, current _: Ice.Current) -> (returnValue: M?, v2: M?) {
        return (v1, v1)
    }

    func opF1(f11: F1?, current _: Ice.Current) -> (returnValue: F1?, f12: F1?) {
        return (f11, F1(name: "F12"))
    }

    func opF2(f21: F2Prx?, current: Current) throws -> (returnValue: F2Prx?, f22: F2Prx?) {
        let prx = try current.adapter.getCommunicator().stringToProxy("F22")!
        return (f21, uncheckedCast(prx: prx, type: F2Prx.self))
    }

    func opF3(f31: F3?, current: Current) throws -> (returnValue: F3?, f32: F3?) {
        let prx = try current.adapter.getCommunicator().stringToProxy("F22")!
        return (f31, F3(f1: F1(name: "F12"), f2: uncheckedCast(prx: prx, type: F2Prx.self)))
    }

    func hasF3(current _: Current) -> Bool {
        return true
    }
}

final class F2I: F2 {
    func op(current _: Current) {}
}

final class UnexpectedObjectExceptionTestDispatcher: Ice.Dispatcher {
    public func dispatch(_ request: IncomingRequest) throws -> OutgoingResponse {
        let ae = AlsoEmpty()
        return request.current.makeOutgoingResponse(
            ae, formatType: nil
        ) { ostr, ae in
            ostr.write(ae)
            ostr.writePendingValues()
        }
    }
}
