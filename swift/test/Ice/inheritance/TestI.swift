// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

class IAI: MAIA, @unchecked Sendable {
    func iaop(p: MAIAPrx?, current _: Ice.Current) -> MAIAPrx? {
        return p
    }

    // Since we plan to reuse this implementation, we need to define an overridable dispatch method in the base class.
    func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

// Tests implementation reuse by deriving from IAI.
class IB1I: IAI, MBIB1, @unchecked Sendable {
    func ib1op(p: MBIB1Prx?, current _: Ice.Current) -> MBIB1Prx? {
        return p
    }

    // Override the dispatch method with the correct implementation.
    override func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

// Tests implementation reuse by deriving from IAI.
class IB2I: IAI, MBIB2, @unchecked Sendable {
    func ib2op(p: MBIB2Prx?, current _: Ice.Current) -> MBIB2Prx? {
        return p
    }

    // Override the dispatch method with the correct implementation.
    override func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

// Tests implementation reuse by deriving from IB1I.
final class ICI: IB1I, MAIC, @unchecked Sendable {
    func ib2op(p: MBIB2Prx?, current _: Ice.Current) -> MBIB2Prx? {
        return p
    }

    func icop(p: MAICPrx?, current _: Ice.Current) -> MAICPrx? {
        return p
    }

    // Override the dispatch method with the correct implementation.
    override func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse {
        // The implementation forwards to the static dispatch method defined on the generated protocol extension.
        try await Self.dispatch(self, request: request)
    }
}

final class InitialI: Initial {
    private let _ia: MAIAPrx
    private let _ib1: MBIB1Prx
    private let _ib2: MBIB2Prx
    private let _ic: MAICPrx

    init(_ adapter: Ice.ObjectAdapter) throws {
        _ia = try uncheckedCast(prx: adapter.addWithUUID(IAI()), type: MAIAPrx.self)
        _ib1 = try uncheckedCast(prx: adapter.addWithUUID(IB1I()), type: MBIB1Prx.self)
        _ib2 = try uncheckedCast(prx: adapter.addWithUUID(IB2I()), type: MBIB2Prx.self)
        _ic = try uncheckedCast(prx: adapter.addWithUUID(ICI()), type: MAICPrx.self)
    }

    func iaop(current _: Ice.Current) -> MAIAPrx? {
        return _ia
    }

    func ib1op(current _: Ice.Current) -> MBIB1Prx? {
        return _ib1
    }

    func ib2op(current _: Ice.Current) -> MBIB2Prx? {
        return _ib2
    }

    func icop(current _: Ice.Current) -> MAICPrx? {
        return _ic
    }

    func shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown()
    }
}
