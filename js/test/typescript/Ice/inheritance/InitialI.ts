//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import {Ice} from "ice";
import {Test} from "./generated";

export class IAI extends Test.MA.IA
{
    iaop(p:Test.MA.IAPrx, current:Ice.Current):Test.MA.IAPrx
    {
        return p;
    }
}

export class IB1I extends Test.MB.IB1
{
    iaop(p:Test.MA.IAPrx, current:Ice.Current):Test.MA.IAPrx
    {
        return p;
    }

    ib1op(p:Test.MB.IB1Prx, current:Ice.Current):Test.MB.IB1Prx
    {
        return p;
    }
}

export class IB2I extends Test.MB.IB2
{
    iaop(p:Test.MA.IAPrx, current:Ice.Current):Test.MA.IAPrx
    {
        return p;
    }

    ib2op(p:Test.MB.IB2Prx, current:Ice.Current):Test.MB.IB2Prx
    {
        return p;
    }
}

export class ICI extends Test.MA.IC
{
    iaop(p:Test.MA.IAPrx, current:Ice.Current):Test.MA.IAPrx
    {
        return p;
    }

    icop(p:Test.MA.ICPrx, current:Ice.Current):Test.MA.ICPrx
    {
        return p;
    }

    ib1op(p:Test.MB.IB1Prx, current:Ice.Current):Test.MB.IB1Prx
    {
        return p;
    }

    ib2op(p:Test.MB.IB2Prx, current:Ice.Current):Test.MB.IB2Prx
    {
        return p;
    }
}

export class InitialI extends Test.Initial
{
    constructor(adapter:Ice.ObjectAdapter, obj:Ice.ObjectPrx)
    {
        super();
        const endpts = obj.ice_getEndpoints();
        this._ia = Test.MA.IAPrx.uncheckedCast(adapter.addWithUUID(new IAI()).ice_endpoints(endpts));
        this._ib1 = Test.MB.IB1Prx.uncheckedCast(adapter.addWithUUID(new IB1I()).ice_endpoints(endpts));
        this._ib2 = Test.MB.IB2Prx.uncheckedCast(adapter.addWithUUID(new IB2I()).ice_endpoints(endpts));
        this._ic = Test.MA.ICPrx.uncheckedCast(adapter.addWithUUID(new ICI()).ice_endpoints(endpts));
    }

    iaop(current:Ice.Current):Test.MA.IAPrx
    {
        return this._ia;
    }

    ib1op(current:Ice.Current):Test.MB.IB1Prx
    {
        return this._ib1;
    }

    ib2op(current:Ice.Current):Test.MB.IB2Prx
    {
        return this._ib2;
    }

    icop(current:Ice.Current):Test.MA.ICPrx
    {
        return this._ic;
    }

    shutdown(current:Ice.Current):void
    {
        current.adapter.getCommunicator().shutdown();
    }

    _ia:Test.MA.IAPrx;
    _ib1:Test.MB.IB1Prx;
    _ib2:Test.MB.IB2Prx;
    _ic:Test.MA.ICPrx;
}
