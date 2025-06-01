// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class IAI: MAIA {
    func iaop(p: MAIAPrx?, current _: Ice.Current) async throws -> MAIAPrx? {
        return p
    }

    // Since we plan to reuse this implementation, we need to define an overridable dispatch method in the base class.
    func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

// Tests implementation reuse by deriving from IAI.
class IB1I: IAI, MBIB1 {
    func ib1op(p: MBIB1Prx?, current _: Ice.Current) async throws -> MBIB1Prx? {
        return p
    }

    // Override the dispatch method with the correct implementation.
    override func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

// Tests implementation reuse by deriving from IAI.
class IB2I: IAI, MBIB2 {
    func ib2op(p: MBIB2Prx?, current _: Ice.Current) async throws -> MBIB2Prx? {
        return p
    }

    // Override the dispatch method with the correct implementation.
    override func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

// Tests implementation reuse by deriving from IB1I.
final class ICI: IB1I, MAIC {
    func ib2op(p: MBIB2Prx?, current _: Ice.Current) async throws -> MBIB2Prx? {
        return p
    }

    func icop(p: MAICPrx?, current _: Ice.Current) async throws -> MAICPrx? {
        return p
    }

    // Override the dispatch method with the correct implementation.
    override func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

class InitialI: Initial {
    let _ia: MAIAPrx
    let _ib1: MBIB1Prx
    let _ib2: MBIB2Prx
    let _ic: MAICPrx

    init(_ adapter: Ice.ObjectAdapter) throws {
        _ia = try uncheckedCast(prx: adapter.addWithUUID(IAI()), type: MAIAPrx.self)
        _ib1 = try uncheckedCast(prx: adapter.addWithUUID(IB1I()), type: MBIB1Prx.self)
        _ib2 = try uncheckedCast(prx: adapter.addWithUUID(IB2I()), type: MBIB2Prx.self)
        _ic = try uncheckedCast(prx: adapter.addWithUUID(ICI()), type: MAICPrx.self)
    }

    func iaop(current _: Ice.Current) async throws -> MAIAPrx? {
        return _ia
    }

    func ib1op(current _: Ice.Current) async throws -> MBIB1Prx? {
        return _ib1
    }

    func ib2op(current _: Ice.Current) async throws -> MBIB2Prx? {
        return _ib2
    }

    func icop(current _: Ice.Current) async throws -> MAICPrx? {
        return _ic
    }

    func shutdown(current: Ice.Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}
