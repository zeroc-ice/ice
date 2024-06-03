//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class IAI: MAIA {
    func iaop(p: MAIAPrx?, current _: Ice.Current) throws -> MAIAPrx? {
        return p
    }
}

class IB1I: MBIB1 {
    func iaop(p: MAIAPrx?, current _: Ice.Current) throws -> MAIAPrx? {
        return p
    }

    func ib1op(p: MBIB1Prx?, current _: Ice.Current) throws -> MBIB1Prx? {
        return p
    }
}

class IB2I: MBIB2 {
    func iaop(p: MAIAPrx?, current _: Ice.Current) throws -> MAIAPrx? {
        return p
    }

    func ib2op(p: MBIB2Prx?, current _: Ice.Current) throws -> MBIB2Prx? {
        return p
    }
}

class ICI: MAIC {
    func iaop(p: MAIAPrx?, current _: Ice.Current) throws -> MAIAPrx? {
        return p
    }

    func icop(p: MAICPrx?, current _: Ice.Current) throws -> MAICPrx? {
        return p
    }

    func ib1op(p: MBIB1Prx?, current _: Ice.Current) throws -> MBIB1Prx? {
        return p
    }

    func ib2op(p: MBIB2Prx?, current _: Ice.Current) throws -> MBIB2Prx? {
        return p
    }
}

class InitialI: Initial {
    let _ia: MAIAPrx
    let _ib1: MBIB1Prx
    let _ib2: MBIB2Prx
    let _ic: MAICPrx

    init(_ adapter: Ice.ObjectAdapter) throws {
        _ia = try uncheckedCast(prx: adapter.addWithUUID(MAIADisp(IAI())), type: MAIAPrx.self)
        _ib1 = try uncheckedCast(prx: adapter.addWithUUID(MBIB1Disp(IB1I())), type: MBIB1Prx.self)
        _ib2 = try uncheckedCast(prx: adapter.addWithUUID(MBIB2Disp(IB2I())), type: MBIB2Prx.self)
        _ic = try uncheckedCast(prx: adapter.addWithUUID(MAICDisp(ICI())), type: MAICPrx.self)
    }

    func iaop(current _: Ice.Current) throws -> MAIAPrx? {
        return _ia
    }

    func ib1op(current _: Ice.Current) throws -> MBIB1Prx? {
        return _ib1
    }

    func ib2op(current _: Ice.Current) throws -> MBIB2Prx? {
        return _ib2
    }

    func icop(current _: Ice.Current) throws -> MAICPrx? {
        return _ic
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter.getCommunicator().shutdown()
    }
}
