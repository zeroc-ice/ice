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
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var Class = Ice.Class;

    var CAI = Class(Test.MA.CA, {
        caop: function(p, current)
        {
            return p;
        }
    });

    var CBI = Class(Test.MB.CB, {
        caop: function(p, current)
        {
            return p;
        },
        cbop: function(p, current)
        {
            return p;
        }
    });

    var CCI = Class(Test.MA.CC, {
        caop: function(p, current)
        {
            return p;
        },
        ccop: function(p, current)
        {
            return p;
        },
        cbop: function(p, current)
        {
            return p;
        }
    });

    var CDI = Class(Test.MA.CD, {
        caop: function(p, current)
        {
            return p;
        },
        ccop: function(p, current)
        {
            return p;
        },
        cdop: function(p, current)
        {
            return p;
        },
        iaop: function(p, current)
        {
            return p;
        },
        cbop: function(p, current)
        {
            return p;
        },
        ib1op: function(p, current)
        {
            return p;
        },
        ib2op: function(p, current)
        {
            return p;
        }
    });

    var IAI = Class(Test.MA.IA, {
        iaop: function(p, current)
        {
            return p;
        }
    });

    var IB1I = Class(Test.MB.IB1, {
        iaop: function(p, current)
        {
            return p;
        },
        ib1op: function(p, current)
        {
            return p;
        }
    });

    var IB2I = Class(Test.MB.IB2, {
        iaop: function(p, current)
        {
            return p;
        },
        ib2op: function(p, current)
        {
            return p;
        }
    });

    var ICI = Class(Test.MA.IC, {
        iaop: function(p, current)
        {
            return p;
        },
        icop: function(p, current)
        {
            return p;
        },
        ib1op: function(p, current)
        {
            return p;
        },
        ib2op: function(p, current)
        {
            return p;
        }
    });

    var InitialI = Class(Test.Initial, {
        __init__: function(adapter, obj)
        {
            var endpts = obj.ice_getEndpoints();
            this._ca = Test.MA.CAPrx.uncheckedCast(adapter.addWithUUID(new CAI()).ice_endpoints(endpts));
            this._cb = Test.MB.CBPrx.uncheckedCast(adapter.addWithUUID(new CBI()).ice_endpoints(endpts));
            this._cc = Test.MA.CCPrx.uncheckedCast(adapter.addWithUUID(new CCI()).ice_endpoints(endpts));
            this._cd = Test.MA.CDPrx.uncheckedCast(adapter.addWithUUID(new CDI()).ice_endpoints(endpts));
            this._ia = Test.MA.IAPrx.uncheckedCast(adapter.addWithUUID(new IAI()).ice_endpoints(endpts));
            this._ib1 = Test.MB.IB1Prx.uncheckedCast(adapter.addWithUUID(new IB1I()).ice_endpoints(endpts));
            this._ib2 = Test.MB.IB2Prx.uncheckedCast(adapter.addWithUUID(new IB2I()).ice_endpoints(endpts));
            this._ic = Test.MA.ICPrx.uncheckedCast(adapter.addWithUUID(new ICI()).ice_endpoints(endpts));
        },
        caop: function(current)
        {
            return this._ca;
        },
        cbop: function(current)
        {
            return this._cb;
        },
        ccop: function(current)
        {
            return this._cc;
        },
        cdop: function(current)
        {
            return this._cd;
        },
        iaop: function(current)
        {
            return this._ia;
        },
        ib1op: function(current)
        {
            return this._ib1;
        },
        ib2op: function(current)
        {
            return this._ib2;
        },
        icop: function(current)
        {
            return this._ic;
        },
        shutdown: function(current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    });

    exports.InitialI = InitialI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
