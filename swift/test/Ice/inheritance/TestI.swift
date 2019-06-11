//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class CAI: MACAOperations {
    func caop(p: MACAPrx?, current: Ice.Current) throws -> MACAPrx? {
        return p
    }
}

class CBI: MBCBOperations {
    func caop(p: MACAPrx?, current: Ice.Current) throws -> MACAPrx? {
        return p
    }

    func cbop(p: MBCBPrx?, current: Ice.Current) throws -> MBCBPrx? {
        return p
    }
}

class CCI: MACCOperations {
    func caop(p: MACAPrx?, current: Ice.Current) throws -> MACAPrx? {
        return p
    }

    func ccop(p: MACCPrx?, current: Ice.Current) throws -> MACCPrx? {
        return p
    }

    func cbop(p: MBCBPrx?, current: Ice.Current) throws -> MBCBPrx? {
        return p
    }
}

class CDI: MACDOperations {
    func caop(p: MACAPrx?, current: Ice.Current) throws -> MACAPrx? {
        return p
    }

    func ccop(p: MACCPrx?, current: Ice.Current) throws -> MACCPrx? {
        return p
    }

    func cdop(p: MACDPrx?, current: Ice.Current) throws -> MACDPrx? {
        return p
    }

    func iaop(p: MAIAPrx?, current: Ice.Current) throws -> MAIAPrx? {
        return p
    }

    func cbop(p: MBCBPrx?, current: Ice.Current) throws -> MBCBPrx? {
        return p
    }

    func ib1op(p: MBIB1Prx?, current: Ice.Current) throws -> MBIB1Prx? {
        return p
    }

    func ib2op(p: MBIB2Prx?, current: Ice.Current) throws -> MBIB2Prx? {
        return p
    }
}

class IAI: MAIA {
    func iaop(p: MAIAPrx?, current: Ice.Current) throws -> MAIAPrx? {
        return p
    }
}

class IB1I: MBIB1 {

    func iaop(p: MAIAPrx?, current: Ice.Current) throws -> MAIAPrx? {
        return p
    }

    func ib1op(p: MBIB1Prx?, current: Ice.Current) throws -> MBIB1Prx? {
        return p
    }
}

class IB2I: MBIB2 {

    func iaop(p: MAIAPrx?, current: Ice.Current) throws -> MAIAPrx? {
        return p
    }

    func ib2op(p: MBIB2Prx?, current: Ice.Current) throws -> MBIB2Prx? {
        return p
    }
}

class ICI: MAIC {

    func iaop(p: MAIAPrx?, current: Ice.Current) throws -> MAIAPrx? {
        return p
    }

    func icop(p: MAICPrx?, current: Ice.Current) throws -> MAICPrx? {
        return p
    }

    func ib1op(p: MBIB1Prx?, current: Ice.Current) throws -> MBIB1Prx? {
        return p
    }

    func ib2op(p: MBIB2Prx?, current: Ice.Current) throws -> MBIB2Prx? {
        return p
    }
}

class InitialI: Initial {
    let _ca: MACAPrx
    let _cb: MBCBPrx
    let _cc: MACCPrx
    let _cd: MACDPrx
    let _ia: MAIAPrx
    let _ib1: MBIB1Prx
    let _ib2: MBIB2Prx
    let _ic: MAICPrx

    init(_ adapter: Ice.ObjectAdapter) throws {
        _ca = try uncheckedCast(prx: adapter.addWithUUID(MACADisp(CAI())), type: MACAPrx.self)
        _cb = try uncheckedCast(prx: adapter.addWithUUID(MBCBDisp(CBI())), type: MBCBPrx.self)
        _cc = try uncheckedCast(prx: adapter.addWithUUID(MACCDisp(CCI())), type: MACCPrx.self)
        _cd = try uncheckedCast(prx: adapter.addWithUUID(MACDDisp(CDI())), type: MACDPrx.self)
        _ia = try uncheckedCast(prx: adapter.addWithUUID(MAIADisp(IAI())), type: MAIAPrx.self)
        _ib1 = try uncheckedCast(prx: adapter.addWithUUID(MBIB1Disp(IB1I())), type: MBIB1Prx.self)
        _ib2 = try uncheckedCast(prx: adapter.addWithUUID(MBIB2Disp(IB2I())), type: MBIB2Prx.self)
        _ic = try uncheckedCast(prx: adapter.addWithUUID(MAICDisp(ICI())), type: MAICPrx.self)
    }

    func caop(current: Ice.Current) throws -> MACAPrx? {
        return _ca
    }

    func cbop(current: Ice.Current) throws -> MBCBPrx? {
        return _cb
    }

    func ccop(current: Ice.Current) throws -> MACCPrx? {
        return _cc
    }

    func cdop(current: Ice.Current) throws -> MACDPrx? {
        return _cd
    }

    func iaop(current: Ice.Current) throws -> MAIAPrx? {
        return _ia
    }

    func ib1op(current: Ice.Current) throws -> MBIB1Prx? {
        return _ib1
    }

    func ib2op(current: Ice.Current) throws -> MBIB2Prx? {
        return _ib2
    }

    func icop(current: Ice.Current) throws -> MAICPrx? {
        return _ic
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
