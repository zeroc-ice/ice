// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************
import {Ice} from "ice";;
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";
const test = TestHelper.test;

export class MyDerivedClassI extends Test.MyDerivedClass
{
    //
    // Override the Object "pseudo" operations to verify the operation mode.
    //
    constructor(endpoints:Ice.Endpoint[])
    {
        super();
        this._opByteSOnewayCount = 0;
        this._endpoints = endpoints;
    }

    ice_isA(id:string, current:Ice.Current):boolean
    {
        test(current.mode === Ice.OperationMode.Nonmutating);
        return Ice.Object.prototype.ice_isA.call(this, id, current);
    }

    ice_ping(current:Ice.Current):void
    {
        test(current.mode === Ice.OperationMode.Nonmutating);
        Ice.Object.prototype.ice_ping.call(this, current);
    }

    ice_ids(current:Ice.Current):Ice.StringSeq
    {
        test(current.mode === Ice.OperationMode.Nonmutating);
        return Ice.Object.prototype.ice_ids.call(this, current);
    }

    ice_id(current:Ice.Current):string
    {
        test(current.mode === Ice.OperationMode.Nonmutating);
        return Ice.Object.prototype.ice_id.call(this, current);
    }

    shutdown(current:Ice.Current):void
    {
        current.adapter.getCommunicator().shutdown();
    }

    supportsCompress(current:Ice.Current):boolean
    {
        return false;
    }

    opVoid(current:Ice.Current):void
    {
        test(current.mode === Ice.OperationMode.Normal);
    }

    opBool(p1:boolean, p2:boolean, current:Ice.Current):[boolean, boolean]
    {
        return [p2, p1];
    }

    opBoolS(p1:Test.BoolS, p2:Test.BoolS, current:Ice.Current):[Test.BoolS, Test.BoolS]
    {
        const p3 = p1.concat(p2);
        return [p1.reverse(), p3];
    }

    opBoolSS(p1:Test.BoolSS, p2:Test.BoolSS, current:Ice.Current):[Test.BoolSS, Test.BoolSS]
    {
        const p3 = p1.concat(p2);
        return [p1.reverse(), p3];
    }

    opByte(p1:number, p2:number, current:Ice.Current):[number, number]
    {
        return [p1, (p1 ^ p2) & 0xff];
    }

    opByteBoolD(p1:Test.ByteBoolD, p2:Test.ByteBoolD, current:Ice.Current):[Test.ByteBoolD, Test.ByteBoolD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opByteS(p1:Test.ByteS, p2:Test.ByteS, current:Ice.Current):[Test.ByteS, Test.ByteS]
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

    opByteSS(p1:Test.ByteSS, p2:Test.ByteSS, current:Ice.Current):[Test.ByteSS, Test.ByteSS]
    {
        const r = p1.concat(p2);
        return [r, p1.reverse()];
    }

    opFloatDouble(p1:number, p2:number, current:Ice.Current):[number, number, number]
    {
        return [p2, p1, p2];
    }

    opFloatDoubleS(p1:Test.FloatS, p2:Test.DoubleS, current:Ice.Current):
        [Test.DoubleS, Test.FloatS, Test.DoubleS]
    {
        const r = p2.concat(p1);
        const p4 = p2.reverse();
        return [r, p1, p4];
    }

    opFloatDoubleSS(p1:Test.FloatSS, p2:Test.DoubleSS, current:Ice.Current):
        [Test.DoubleSS, Test.FloatSS, Test.DoubleSS]
    {
        const r = p2.concat(p2);
        const p4 = p2.reverse();
        return [r, p1, p4];
    }

    opLongFloatD(p1:Test.LongFloatD, p2:Test.LongFloatD, current:Ice.Current):[Test.LongFloatD, Test.LongFloatD]
    {
        const r = new Ice.HashMap(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opMyClass(p1:Test.MyClassPrx, current:Ice.Current):[Test.MyClassPrx, Test.MyClassPrx, Test.MyClassPrx]
    {
        const p2 = p1;
        const p3 = Test.MyClassPrx.uncheckedCast(
            current.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")));
        const r = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id));
        return [r.ice_endpoints(this._endpoints), p2, p3.ice_endpoints(this._endpoints)];
    }

    opMyEnum(p1:Test.MyEnum, current:Ice.Current):[Test.MyEnum, Test.MyEnum]
    {
        return [Test.MyEnum.enum3, p1];
    }

    opShortIntD(p1:Test.ShortIntD, p2:Test.ShortIntD, current:Ice.Current):[Test.ShortIntD, Test.ShortIntD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opShortIntLong(p1:number, p2:number, p3:Ice.Long, current:Ice.Current):
        [Ice.Long, number, number, Ice.Long]
    {
        return [p3, p1, p2, p3];
    }

    opShortIntLongS(p1:Test.ShortS, p2:Test.IntS, p3:Test.LongS, current:Ice.Current):
        [Test.LongS, Test.ShortS, Test.IntS, Test.LongS]
    {
        return [p3, p1, p2.reverse(), p3.concat(p3)];
    }

    opShortIntLongSS(p1:Test.ShortSS, p2:Test.IntSS, p3:Test.LongSS, current:Ice.Current):
        [Test.LongSS, Test.ShortSS, Test.IntSS, Test.LongSS]
    {
        return [p3, p1, p2.reverse(), p3.concat(p3)];
    }

    opString(p1:string, p2:string, current:Ice.Current):[string, string]
    {
        return [p1 + " " + p2, p2 + " " + p1];
    }

    opStringMyEnumD(p1:Test.StringMyEnumD,
                    p2:Test.StringMyEnumD, current:Ice.Current):[Test.StringMyEnumD, Test.StringMyEnumD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opMyEnumStringD(p1:Test.MyEnumStringD,
                    p2:Test.MyEnumStringD, current:Ice.Current):[Test.MyEnumStringD, Test.MyEnumStringD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opMyStructMyEnumD(p1:Test.MyStructMyEnumD,
                      p2:Test.MyStructMyEnumD, current:Ice.Current):[Test.MyStructMyEnumD, Test.MyStructMyEnumD]
    {
        const r = new Ice.HashMap(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opByteBoolDS(p1:Test.ByteBoolDS, p2:Test.ByteBoolDS, current:Ice.Current):[Test.ByteBoolDS, Test.ByteBoolDS]
    {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opShortIntDS(p1:Test.ShortIntDS, p2:Test.ShortIntDS, current:Ice.Current):[Test.ShortIntDS, Test.ShortIntDS]
    {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opLongFloatDS(p1:Test.LongFloatDS, p2:Test.LongFloatDS, current:Ice.Current):[Test.LongFloatDS, Test.LongFloatDS]
    {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opStringStringDS(p1:Test.StringStringDS,
                     p2:Test.StringStringDS, current:Ice.Current):[Test.StringStringDS, Test.StringStringDS]
    {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opStringMyEnumDS(p1:Test.StringMyEnumDS,
                     p2:Test.StringMyEnumDS, current:Ice.Current):[Test.StringMyEnumDS, Test.StringMyEnumDS]
    {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opMyEnumStringDS(p1:Test.MyEnumStringDS,
                     p2:Test.MyEnumStringDS, current:Ice.Current):[Test.MyEnumStringDS, Test.MyEnumStringDS]
    {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opMyStructMyEnumDS(p1:Test.MyStructMyEnumDS,
                       p2:Test.MyStructMyEnumDS, current:Ice.Current):[Test.MyStructMyEnumDS, Test.MyStructMyEnumDS]
    {
        const p3 = p2.concat(p1);
        const r = p1.reverse();
        return [r, p3];
    }

    opByteByteSD(p1:Test.ByteByteSD, p2:Test.ByteByteSD, current:Ice.Current):[Test.ByteByteSD, Test.ByteByteSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opBoolBoolSD(p1:Test.BoolBoolSD, p2:Test.BoolBoolSD, current:Ice.Current):[Test.BoolBoolSD, Test.BoolBoolSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opShortShortSD(p1:Test.ShortShortSD, p2:Test.ShortShortSD, current:Ice.Current):[Test.ShortShortSD, Test.ShortShortSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opIntIntSD(p1:Test.IntIntSD, p2:Test.IntIntSD, current:Ice.Current):[Test.IntIntSD, Test.IntIntSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opLongLongSD(p1:Test.LongLongSD, p2:Test.LongLongSD, current:Ice.Current):[Test.LongLongSD, Test.LongLongSD]
    {
        const r = new Ice.HashMap(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Ice.HashMap(p2);
        return [r, p3];
    }

    opStringFloatSD(p1:Test.StringFloatSD,
                    p2:Test.StringFloatSD, current:Ice.Current):[Test.StringFloatSD, Test.StringFloatSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opStringDoubleSD(p1:Test.StringDoubleSD,
                     p2:Test.StringDoubleSD, current:Ice.Current):[Test.StringDoubleSD, Test.StringDoubleSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opStringStringSD(p1:Test.StringStringSD,
                     p2:Test.StringStringSD, current:Ice.Current):[Test.StringStringSD, Test.StringStringSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opMyEnumMyEnumSD(p1:Test.MyEnumMyEnumSD,
                     p2:Test.MyEnumMyEnumSD, current:Ice.Current):[Test.MyEnumMyEnumSD, Test.MyEnumMyEnumSD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        const p3 = new Map(p2);
        return [r, p3];
    }

    opIntS(s:Test.IntS, current:Ice.Current):Test.IntS
    {
        return s.map(v => -v);
    }

    opByteSOneway(s:Test.ByteS, current:Ice.Current):void
    {
        this._opByteSOnewayCount += 1;
    }

    opByteSOnewayCallCount(current:Ice.Current):number
    {
        const count = this._opByteSOnewayCount;
        this._opByteSOnewayCount = 0;
        return count;
    }

    opContext(current:Ice.Current):Ice.Context
    {
        return current.ctx;
    }

    opDoubleMarshaling(p1:number, p2:Test.DoubleS, current:Ice.Current):void
    {
        const d = 1278312346.0 / 13.0;
        test(p1 === d);
        for(let i = 0; i < p2.length; ++i)
        {
            test(p2[i] === d);
        }
    }

    opStringS(p1:Test.StringS, p2:Test.StringS, current:Ice.Current):[Test.StringS, Test.StringS]
    {
        const p3 = p1.concat(p2);
        const r = p1.reverse();
        return [r, p3];
    }

    opStringSS(p1:Test.StringSS, p2:Test.StringSS, current:Ice.Current):[Test.StringSS, Test.StringSS]
    {
        const p3 = p1.concat(p2);
        const r = p2.reverse();
        return [r, p3];
    }

    opStringSSS(p1:Test.StringSSS, p2:Test.StringSSS, current:Ice.Current):[Test.StringSSS, Test.StringSSS]
    {
        const p3 = p1.concat(p2);
        const r = p2.reverse();
        return [r, p3];
    }

    opStringStringD(p1:Test.StringStringD, p2:Test.StringStringD, current:Ice.Current):[Test.StringStringD, Test.StringStringD]
    {
        const r = new Map(p1);
        p2.forEach((value, key) => r.set(key, value));
        return [r, p1];
    }

    opStruct(p1:Test.Structure, p2:Test.Structure, current:Ice.Current):[Test.Structure, Test.Structure]
    {
        p1.s.s = "a new string";
        return [p2, p1];
    }

    opIdempotent(current:Ice.Current):void
    {
        test(current.mode === Ice.OperationMode.Idempotent);
    }

    opNonmutating(current:Ice.Current):void
    {
        test(current.mode === Ice.OperationMode.Nonmutating);
    }

    opDerived(current:Ice.Current):void
    {
    }

    opByte1(value:number, current:Ice.Current):number
    {
        return value;
    }

    opShort1(value:number, current:Ice.Current):number
    {
        return value;
    }

    opInt1(value:number, current:Ice.Current):number
    {
        return value;
    }

    opLong1(value:Ice.Long, current:Ice.Current):Ice.Long
    {
        return value;
    }

    opFloat1(value:number, current:Ice.Current):number
    {
        return value;
    }

    opDouble1(value:number, current:Ice.Current):number
    {
        return value;
    }

    opString1(value:string, current:Ice.Current):string
    {
        return value;
    }

    opStringS1(value:Test.StringS, current:Ice.Current):Test.StringS
    {
        return value;
    }

    opByteBoolD1(value:Test.ByteBoolD, current:Ice.Current):Test.ByteBoolD
    {
        return value;
    }

    opStringS2(value:Test.StringS, current:Ice.Current):Test.StringS
    {
        return value;
    }

    opByteBoolD2(value:Test.ByteBoolD, current:Ice.Current):Test.ByteBoolD
    {
        return value;
    }

    opMyClass1(value:Test.MyClass1, current:Ice.Current):Test.MyClass1
    {
        return value;
    }

    opMyStruct1(value:Test.MyStruct1, current:Ice.Current):Test.MyStruct1
    {
        return value;
    }

    opStringLiterals(current:Ice.Current):Test.StringS
    {
        return [
            Test.s0, Test.s1, Test.s2, Test.s3, Test.s4, Test.s5, Test.s6, Test.s7, Test.s8, Test.s9, Test.s10,
            Test.sw0, Test.sw1, Test.sw2, Test.sw3, Test.sw4, Test.sw5, Test.sw6, Test.sw7, Test.sw8, Test.sw9, Test.sw10,
            Test.ss0, Test.ss1, Test.ss2, Test.ss3, Test.ss4, Test.ss5,
            Test.su0, Test.su1, Test.su2
        ];
    }

    opWStringLiterals(current:Ice.Current):Test.WStringS
    {
        return this.opStringLiterals(current);
    }

    opMStruct1(current:Ice.Current):Test.Structure
    {
        return new Test.Structure();
    }

    opMStruct2(p1:Test.Structure, current:Ice.Current):[Test.Structure, Test.Structure]
    {
        return [p1, p1];
    }

    opMSeq1(current:Ice.Current):Test.StringS
    {
        return [];
    }

    opMSeq2(p1:Test.StringS, current:Ice.Current):[Test.StringS, Test.StringS]
    {
        return [p1, p1];
    }

    opMDict1(current:Ice.Current):Test.StringStringD
    {
        return new Map();
    }

    opMDict2(p1:Test.StringStringD, current:Ice.Current):[Test.StringStringD, Test.StringStringD]
    {
        return [p1, p1];
    }

    _opByteSOnewayCount:number;
    _endpoints:Ice.Endpoint[];
}
