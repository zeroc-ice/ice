//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Test } from "./Test.js";

export class IAI extends Test.MA.IA
{
    iaop(p, current)
    {
        return p;
    }
}

export class IB1I extends Test.MB.IB1
{
    iaop(p, current)
    {
        return p;
    }

    ib1op(p, current)
    {
        return p;
    }
}

export class IB2I extends Test.MB.IB2
{
    iaop(p, current)
    {
        return p;
    }

    ib2op(p, current)
    {
        return p;
    }
}

export class ICI extends Test.MA.IC
{
    iaop(p, current)
    {
        return p;
    }

    icop(p, current)
    {
        return p;
    }

    ib1op(p, current)
    {
        return p;
    }

    ib2op(p, current)
    {
        return p;
    }
}

export class InitialI extends Test.Initial
{
    constructor(adapter, obj)
    {
        super();
        const endpts = obj.ice_getEndpoints();
        this._ia = Test.MA.IAPrx.uncheckedCast(adapter.addWithUUID(new IAI()).ice_endpoints(endpts));
        this._ib1 = Test.MB.IB1Prx.uncheckedCast(adapter.addWithUUID(new IB1I()).ice_endpoints(endpts));
        this._ib2 = Test.MB.IB2Prx.uncheckedCast(adapter.addWithUUID(new IB2I()).ice_endpoints(endpts));
        this._ic = Test.MA.ICPrx.uncheckedCast(adapter.addWithUUID(new ICI()).ice_endpoints(endpts));
    }

    iaop(current)
    {
        return this._ia;
    }

    ib1op(current)
    {
        return this._ib1;
    }

    ib2op(current)
    {
        return this._ib2;
    }

    icop(current)
    {
        return this._ic;
    }

    shutdown(current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
