// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var MyDerivedClassI = Class(Test.MyDerivedClass, {
        //
        // Override the Object "pseudo" operations to verify the operation mode.
        //
        __init__: function()
        {
            this._opByteSOnewayCount = 0;
        },
        ice_isA: function(id, current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_isA.call(this, id, current);
        },
        ice_ping: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            Ice.Object.prototype.ice_ping.call(this, current);
        },
        ice_ids: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_ids.call(this, current);
        },
        ice_id: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_id.call(this, current);
        },
        shutdown: function(current)
        {
            current.adapter.getCommunicator().shutdown();
        },
        delay: function(cb, ms, current)
        {
            Ice.Timer.setTimeout(
                function()
                {
                    cb.ice_response();
                }, ms);
        },
        opVoid: function(current)
        {
            test(current.mode === Ice.OperationMode.Normal);
        },
        opBool: function(p1, p2, current)
        {
            return [p2, p1];
        },
        opBoolS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            return [p1.reverse(), p3];
        },
        opBoolSS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            return [p1.reverse(), p3];
        },
        opByte: function(p1, p2, current)
        {
            return [p1, (p1 ^ p2) & 0xff];
        },
        opByteBoolD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },
        opByteS: function(p1, p2, current)
        {
            var i;
            var p3 = Ice.Buffer.createNative(p1.length);
            for(i = 0; i < p1.length; i++)
            {
                p3[i] = p1[p1.length - (i + 1)];
            }

            var r = Ice.Buffer.createNative(p1.length + p2.length);
            for(i = 0; i < p1.length; ++i)
            {
                r[i] = p1[i];
            }
            for(i = 0; i < p2.length; ++i)
            {
                r[i + p1.length] = p2[i];
            }
            return [r, p3];
        },
        opByteSS: function(p1, p2, current)
        {
            var r = p1.concat(p2);
            return [r, p1.reverse()];
        },
        opFloatDouble: function(p1, p2, current)
        {
            return [p2, p1, p2];
        },
        opFloatDoubleS: function(p1, p2, current)
        {
            var r = p2.concat(p1);
            var p4 = p2.reverse();
            return [r, p1, p4];
        },
        opFloatDoubleSS: function(p1, p2, current)
        {
            var r = p2.concat(p2);
            var p4 = p2.reverse();
            return [r, p1, p4];
        },
        opLongFloatD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },
        opMyClass: function(p1, current)
        {
            var p2 = p1;
            var p3 = Test.MyClassPrx.uncheckedCast(
                current.adapter.createProxy(current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
            var r = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id));
            return [r, p2, p3];
        },
        opMyEnum: function(p1, current)
        {
            return [Test.MyEnum.enum3, p1];
        },
        opShortIntD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },
        opShortIntLong: function(p1, p2, p3, current)
        {
            return [p3, p1, p2, p3];
        },
        opShortIntLongS: function(p1, p2, p3, current)
        {
            return [p3, p1, p2.reverse(), p3.concat(p3)];
        },
        opShortIntLongSS: function(p1, p2, p3, current)
        {
            return [p3, p1, p2.reverse(), p3.concat(p3)];
        },
        opString: function(p1, p2, current)
        {
            return [p1 + " " + p2, p2 + " " + p1];
        },
        opStringMyEnumD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },
        opMyEnumStringD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },
        opMyStructMyEnumD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },
        opByteBoolDS: function(p1, p2, current)
        {
            var p3 = p2.concat(p1);
            var r = p1.reverse();
            return [r, p3];
        },
        opShortIntDS: function(p1, p2, current)
        {
            var p3 = p2.concat(p1);
            var r = p1.reverse();
            return [r, p3];
        },
        opLongFloatDS: function(p1, p2, current)
        {
            var p3 = p2.concat(p1);
            var r = p1.reverse();
            return [r, p3];
        },
        opStringStringDS: function(p1, p2, current)
        {
            var p3 = p2.concat(p1);
            var r = p1.reverse();
            return [r, p3];
        },
        opStringMyEnumDS: function(p1, p2, current)
        {
            var p3 = p2.concat(p1);
            var r = p1.reverse();
            return [r, p3];
        },
        opMyEnumStringDS: function(p1, p2, current)
        {
            var p3 = p2.concat(p1);
            var r = p1.reverse();
            return [r, p3];
        },
        opMyStructMyEnumDS: function(p1, p2, current)
        {
            var p3 = p2.concat(p1);
            var r = p1.reverse();
            return [r, p3];
        },
        opByteByteSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opBoolBoolSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opShortShortSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opIntIntSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opLongLongSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opStringFloatSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opStringDoubleSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opStringStringSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opMyEnumMyEnumSD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            var p3 = p2.clone();
            return [r, p3];
        },
        opIntS: function(s, current)
        {
            return s.map(function(v, i, arr) { return -v; });
        },
        opByteSOneway: function(s, current)
        {
            this._opByteSOnewayCount += 1;
        },
        opByteSOnewayCallCount: function(current)
        {
            var count = this._opByteSOnewayCount;
            this._opByteSOnewayCount = 0;
            return count;
        },
        opContext: function(current)
        {
            return current.ctx;
        },
        opDoubleMarshaling: function(p1, p2, current)
        {
            var d = 1278312346.0 / 13.0;
            test(p1 === d);
            for(var i = 0; i < p2.length; ++i)
            {
                test(p2[i] === d);
            }
        },
        opStringS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            var r = p1.reverse();
            return [r, p3];
        },
        opStringSS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            var r = p2.reverse();
            return [r, p3];
        },
        opStringSSS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            var r = p2.reverse();
            return [r, p3];
        },
        opStringStringD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },
        opStruct: function(p1, p2, current)
        {
            p1.s.s = "a new string";
            return [p2, p1];
        },
        opIdempotent: function(current)
        {
            test(current.mode === Ice.OperationMode.Idempotent);
        },
        opNonmutating: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
        },
        opDerived: function(current)
        {
        },
        opByte1: function(value, current)
        {
            return value;
        },
        opShort1: function(value, current)
        {
            return value;
        },
        opInt1: function(value, current)
        {
            return value;
        },
        opLong1: function(value, current)
        {
            return value;
        },
        opFloat1: function(value, current)
        {
            return value;
        },
        opDouble1: function(value, current)
        {
            return value;
        },
        opString1: function(value, current)
        {
            return value;
        },
        opStringS1: function(value, current)
        {
            return value;
        },
        opByteBoolD1: function(value, current)
        {
            return value;
        },
        opStringS2: function(value, current)
        {
            return value;
        },
        opByteBoolD2: function(value, current)
        {
            return value;
        },
        opMyClass1: function(value, current)
        {
            return value;
        },
        opMyStruct1: function(value, current)
        {
            return value;
        },
        opStringLiterals: function(current)
        {
            return [
                Test.s0, Test.s1, Test.s2, Test.s3, Test.s4, Test.s5, Test.s6, Test.s7, Test.s8, Test.s9, Test.s10,
                Test.sw0, Test.sw1, Test.sw2, Test.sw3, Test.sw4, Test.sw5, Test.sw6, Test.sw7, Test.sw8, Test.sw9, Test.sw10,
                Test.ss0, Test.ss1, Test.ss2, Test.ss3, Test.ss4, Test.ss5,
                Test.su0, Test.su1, Test.su2];
        }
    });

    exports.MyDerivedClassI = MyDerivedClassI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
