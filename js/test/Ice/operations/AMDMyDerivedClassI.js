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
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    function test(value)
    {
        if(!value)
        {
            throw new Error("test failed");
        }
    }

    class AMDMyDerivedClassI extends Test.MyDerivedClass
    {
        //
        // Override the Object "pseudo" operations to verify the operation mode.
        //
        constructor(endpoints)
        {
            super();
            this._opByteSOnewayCount = 0;
            this._endpoints = endpoints;
        }

        ice_isA(id, current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_isA.call(this, id, current);
        }

        ice_ping(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            Ice.Object.prototype.ice_ping.call(this, current);
        }

        ice_ids(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_ids.call(this, current);
        }

        ice_id(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_id.call(this, current);
        }

        async shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        async opVoid(current)
        {
            test(current.mode === Ice.OperationMode.Normal);
        }

        async opBool(p1, p2, current)
        {
            return [p2, p1];
        }

        async opBoolS(p1, p2, current)
        {
            const p3 = p1.concat(p2);
            return [p1.reverse(), p3];
        }

        async opBoolSS(p1, p2, current)
        {
            const p3 = p1.concat(p2);
            return [p1.reverse(), p3];
        }

        async opByte(p1, p2, current)
        {
            return [p1, (p1 ^ p2) & 0xff];
        }

        async opByteBoolD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            return [r, p1];
        }

        async opByteS(p1, p2, current)
        {
            const p3 = new Uint8Array(p1.length);
            for(let i = 0; i < p1.length; i++)
            {
                p3[i] = p1[p1.length - (i + 1)];
            }

            const r = new Uint8Array(p1.length + p2.length);
            for(let i = 0; i < p1.length; ++i)
            {
                r[i] = p1[i];
            }
            for(let i = 0; i < p2.length; ++i)
            {
                r[i + p1.length] = p2[i];
            }
            return [r, p3];
        }

        async opByteSS(p1, p2, current)
        {
            const r = p1.concat(p2);
            return [r, p1.reverse()];
        }

        async opFloatDouble(p1, p2, current)
        {
            return [p2, p1, p2];
        }

        async opFloatDoubleS(p1, p2, current)
        {
            const r = p2.concat(p1);
            const p4 = p2.reverse();
            return [r, p1, p4];
        }

        async opFloatDoubleSS(p1, p2, current)
        {
            const r = p2.concat(p2);
            const p4 = p2.reverse();
            return [r, p1, p4];
        }

        async opLongFloatD(p1, p2, current)
        {
            const r = new Ice.HashMap(p1);
            p2.forEach((value, key) => r.set(key, value));
            return [r, p1];
        }

        async opMyClass(p1, current)
        {
            const p2 = p1;
            const p3 = Test.MyClassPrx.uncheckedCast(
                current.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")));
            const r = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id));
            return [r.ice_endpoints(this._endpoints), p2, p3.ice_endpoints(this._endpoints)];
        }

        async opMyEnum(p1, current)
        {
            return [Test.MyEnum.enum3, p1];
        }

        async opShortIntD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            return [r, p1];
        }

        async opShortIntLong(p1, p2, p3, current)
        {
            return [p3, p1, p2, p3];
        }

        async opShortIntLongS(p1, p2, p3, current)
        {
            return [p3, p1, p2.reverse(), p3.concat(p3)];
        }

        async opShortIntLongSS(p1, p2, p3, current)
        {
            return [p3, p1, p2.reverse(), p3.concat(p3)];
        }

        async opString(p1, p2, current)
        {
            return [p1 + " " + p2, p2 + " " + p1];
        }

        async opStringMyEnumD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            return [r, p1];
        }

        async opMyEnumStringD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            return [r, p1];
        }

        async opMyStructMyEnumD(p1, p2, current)
        {
            const r = new Ice.HashMap(p1);
            p2.forEach((value, key) => r.set(key, value));
            return [r, p1];
        }

        async opByteBoolDS(p1, p2, current)
        {
            const p3 = p2.concat(p1);
            const r = p1.reverse();
            return [r, p3];
        }

        async opShortIntDS(p1, p2, current)
        {
            const p3 = p2.concat(p1);
            const r = p1.reverse();
            return [r, p3];
        }

        async opLongFloatDS(p1, p2, current)
        {
            const p3 = p2.concat(p1);
            const r = p1.reverse();
            return [r, p3];
        }

        async opStringStringDS(p1, p2, current)
        {
            const p3 = p2.concat(p1);
            const r = p1.reverse();
            return [r, p3];
        }

        async opStringMyEnumDS(p1, p2, current)
        {
            const p3 = p2.concat(p1);
            const r = p1.reverse();
            return [r, p3];
        }

        async opMyEnumStringDS(p1, p2, current)
        {
            const p3 = p2.concat(p1);
            const r = p1.reverse();
            return [r, p3];
        }

        async opMyStructMyEnumDS(p1, p2, current)
        {
            const p3 = p2.concat(p1);
            const r = p1.reverse();
            return [r, p3];
        }

        async opByteByteSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opBoolBoolSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opShortShortSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opIntIntSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opLongLongSD(p1, p2, current)
        {
            const r = new Ice.HashMap(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Ice.HashMap(p2);
            return [r, p3];
        }

        async opStringFloatSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opStringDoubleSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opStringStringSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opMyEnumMyEnumSD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            const p3 = new Map(p2);
            return [r, p3];
        }

        async opIntS(s, current)
        {
            return s.map(v => -v);
        }

        async opByteSOneway(s, current)
        {
            this._opByteSOnewayCount += 1;
        }

        async opByteSOnewayCallCount(current)
        {
            const count = this._opByteSOnewayCount;
            this._opByteSOnewayCount = 0;
            return count;
        }

        async opContext(current)
        {
            return current.ctx;
        }

        async opDoubleMarshaling(p1, p2, current)
        {
            const d = 1278312346.0 / 13.0;
            test(p1 === d);
            for(let i = 0; i < p2.length; ++i)
            {
                test(p2[i] === d);
            }
        }

        async opStringS(p1, p2, current)
        {
            const p3 = p1.concat(p2);
            const r = p1.reverse();
            return [r, p3];
        }

        async opStringSS(p1, p2, current)
        {
            const p3 = p1.concat(p2);
            const r = p2.reverse();
            return [r, p3];
        }

        async opStringSSS(p1, p2, current)
        {
            const p3 = p1.concat(p2);
            const r = p2.reverse();
            return [r, p3];
        }

        async opStringStringD(p1, p2, current)
        {
            const r = new Map(p1);
            p2.forEach((value, key) => r.set(key, value));
            return [r, p1];
        }

        async opStruct(p1, p2, current)
        {
            p1.s.s = "a new string";
            return [p2, p1];
        }

        async opIdempotent(current)
        {
            test(current.mode === Ice.OperationMode.Idempotent);
        }

        async opNonmutating(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
        }

        async opDerived(current)
        {
        }

        async opByte1(value, current)
        {
            return value;
        }

        async opShort1(value, current)
        {
            return value;
        }

        async opInt1(value, current)
        {
            return value;
        }

        async opLong1(value, current)
        {
            return value;
        }

        async opFloat1(value, current)
        {
            return value;
        }

        async opDouble1(value, current)
        {
            return value;
        }

        async opString1(value, current)
        {
            return value;
        }

        async opStringS1(value, current)
        {
            return value;
        }

        async opByteBoolD1(value, current)
        {
            return value;
        }

        async opStringS2(value, current)
        {
            return value;
        }

        async opByteBoolD2(value, current)
        {
            return value;
        }

        async opMyClass1(value, current)
        {
            return value;
        }

        async opMyStruct1(value, current)
        {
            return value;
        }

        async opStringLiterals(current)
        {
            return [
                Test.s0, Test.s1, Test.s2, Test.s3, Test.s4, Test.s5, Test.s6, Test.s7, Test.s8, Test.s9, Test.s10,
                Test.sw0, Test.sw1, Test.sw2, Test.sw3, Test.sw4, Test.sw5, Test.sw6, Test.sw7, Test.sw8, Test.sw9, Test.sw10,
                Test.ss0, Test.ss1, Test.ss2, Test.ss3, Test.ss4, Test.ss5,
                Test.su0, Test.su1, Test.su2];
        }

        opWStringLiterals(current)
        {
            return this.opStringLiterals(current);
        }

        async opMStruct1(current)
        {
            return new Test.Structure();
        }

        async opMStruct2(p1, current)
        {
            return [p1, p1];
        }

        async opMSeq1(current)
        {
            return [];
        }

        async opMSeq2(p1, current)
        {
            return [p1, p1];
        }

        async opMDict1(current)
        {
            return new Map();
        }

        async opMDict2(p1, current)
        {
            return [p1, p1];
        }
    }

    exports.AMDMyDerivedClassI = AMDMyDerivedClassI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
