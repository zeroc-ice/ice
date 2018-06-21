// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Test = require("Test").Test;

    class CAI extends Test.MA.CADisp
    {
        caop(p, current)
        {
            return p;
        }
    }

    class CBI extends Test.MB.CBDisp
    {
        caop(p, current)
        {
            return p;
        }

        cbop(p, current)
        {
            return p;
        }
    }

    class CCI extends Test.MA.CCDisp
    {
        caop(p, current)
        {
            return p;
        }

        ccop(p, current)
        {
            return p;
        }

        cbop(p, current)
        {
            return p;
        }
    }

    class CDI extends Test.MA.CDDisp
    {
        caop(p, current)
        {
            return p;
        }

        ccop(p, current)
        {
            return p;
        }

        cdop(p, current)
        {
            return p;
        }

        iaop(p, current)
        {
            return p;
        }

        cbop(p, current)
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

    class IAI extends Test.MA.IA
    {
        iaop(p, current)
        {
            return p;
        }
    }

    class IB1I extends Test.MB.IB1
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

    class IB2I extends Test.MB.IB2
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

    class ICI extends Test.MA.IC
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

    class InitialI extends Test.Initial
    {
        constructor(adapter, obj)
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

        caop(current)
        {
            return this._ca;
        }

        cbop(current)
        {
            return this._cb;
        }

        ccop(current)
        {
            return this._cc;
        }

        cdop(current)
        {
            return this._cd;
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

    exports.InitialI = InitialI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
