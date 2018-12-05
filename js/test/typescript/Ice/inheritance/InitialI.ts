// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated";

export class CAI extends Test.MA.CADisp
{
    caop(p:Test.MA.CAPrx, current:Ice.Current):Test.MA.CAPrx
    {
        return p;
    }
}

export class CBI extends Test.MB.CBDisp
{
    caop(p:Test.MA.CAPrx, current:Ice.Current):Test.MA.CAPrx
    {
        return p;
    }

    cbop(p:Test.MB.CBPrx, current:Ice.Current):Test.MB.CBPrx
    {
        return p;
    }
}

export class CCI extends Test.MA.CCDisp
{
    caop(p:Test.MA.CAPrx, current:Ice.Current):Test.MA.CAPrx
    {
        return p;
    }

    cbop(p:Test.MB.CBPrx, current:Ice.Current):Test.MB.CBPrx
    {
        return p;
    }

    ccop(p:Test.MA.CCPrx, current:Ice.Current):Test.MA.CCPrx
    {
        return p;
    }
}

export class CDI extends Test.MA.CDDisp
{
    caop(p:Test.MA.CAPrx, current:Ice.Current)
    {
        return p;
    }

    ccop(p:Test.MA.CCPrx, current:Ice.Current):Test.MA.CCPrx
    {
        return p;
    }

    cdop(p:Test.MA.CDPrx, current:Ice.Current):Test.MA.CDPrx
    {
        return p;
    }

    iaop(p:Test.MA.IAPrx, current:Ice.Current):Test.MA.IAPrx
    {
        return p;
    }

    cbop(p:Test.MB.CBPrx, current:Ice.Current):Test.MB.CBPrx
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
        this._ca = Test.MA.CAPrx.uncheckedCast(adapter.addWithUUID(new CAI()).ice_endpoints(endpts));
        this._cb = Test.MB.CBPrx.uncheckedCast(adapter.addWithUUID(new CBI()).ice_endpoints(endpts));
        this._cc = Test.MA.CCPrx.uncheckedCast(adapter.addWithUUID(new CCI()).ice_endpoints(endpts));
        this._cd = Test.MA.CDPrx.uncheckedCast(adapter.addWithUUID(new CDI()).ice_endpoints(endpts));
        this._ia = Test.MA.IAPrx.uncheckedCast(adapter.addWithUUID(new IAI()).ice_endpoints(endpts));
        this._ib1 = Test.MB.IB1Prx.uncheckedCast(adapter.addWithUUID(new IB1I()).ice_endpoints(endpts));
        this._ib2 = Test.MB.IB2Prx.uncheckedCast(adapter.addWithUUID(new IB2I()).ice_endpoints(endpts));
        this._ic = Test.MA.ICPrx.uncheckedCast(adapter.addWithUUID(new ICI()).ice_endpoints(endpts));
    }

    caop(current:Ice.Current):Test.MA.CAPrx
    {
        return this._ca;
    }

    cbop(current:Ice.Current):Test.MB.CBPrx
    {
        return this._cb;
    }

    ccop(current:Ice.Current):Test.MA.CCPrx
    {
        return this._cc;
    }

    cdop(current:Ice.Current):Test.MA.CDPrx
    {
        return this._cd;
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

    _ca:Test.MA.CAPrx;
    _cb:Test.MB.CBPrx;
    _cc:Test.MA.CCPrx;
    _cd:Test.MA.CDPrx;
    _ia:Test.MA.IAPrx;
    _ib1:Test.MB.IB1Prx;
    _ib2:Test.MB.IB2Prx;
    _ic:Test.MA.ICPrx;
}
