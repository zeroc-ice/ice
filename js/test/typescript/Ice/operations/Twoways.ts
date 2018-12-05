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
import {TestHelper} from "../../../Common/TestHelper";
const test = TestHelper.test;

export async function run(communicator:Ice.Communicator, prx:Test.MyClassPrx, bidir:boolean, helper:TestHelper)
{
    const literals = await prx.opStringLiterals();

    test(Test.s0 == "\\" &&
            Test.s0 == Test.sw0 &&
            Test.s0 == literals[0] &&
            Test.s0 == literals[11]);

    test(Test.s1 == "A" &&
            Test.s1 == Test.sw1 &&
            Test.s1 == literals[1] &&
            Test.s1 == literals[12]);

    test(Test.s2 == "Ice" &&
            Test.s2 == Test.sw2 &&
            Test.s2 == literals[2] &&
            Test.s2 == literals[13]);

    test(Test.s3 == "A21" &&
            Test.s3 == Test.sw3 &&
            Test.s3 == literals[3] &&
            Test.s3 == literals[14]);

    test(Test.s4 == "\\u0041 \\U00000041" &&
            Test.s4 == Test.sw4 &&
            Test.s4 == literals[4] &&
            Test.s4 == literals[15]);

    test(Test.s5 == "\u00FF" &&
            Test.s5 == Test.sw5 &&
            Test.s5 == literals[5] &&
            Test.s5 == literals[16]);

    test(Test.s6 == "\u03FF" &&
            Test.s6 == Test.sw6 &&
            Test.s6 == literals[6] &&
            Test.s6 == literals[17]);

    test(Test.s7 == "\u05F0" &&
            Test.s7 == Test.sw7 &&
            Test.s7 == literals[7] &&
            Test.s7 == literals[18]);

    test(Test.s8 == "\uD800\uDC00" &&
            Test.s8 == Test.sw8 &&
            Test.s8 == literals[8] &&
            Test.s8 == literals[19]);

    test(Test.s9 == "\uD83C\uDF4C" &&
            Test.s9 == Test.sw9 &&
            Test.s9 == literals[9] &&
            Test.s9 == literals[20]);

    test(Test.s10 == "\u0DA7" &&
            Test.s10 == Test.sw10 &&
            Test.s10 == literals[10] &&
            Test.s10 == literals[21]);

    test(Test.ss0 == "'\"?\\\u0007\b\f\n\r\t\v\u0006" &&
            Test.ss0 == Test.ss1 &&
            Test.ss0 == Test.ss2 &&
            Test.ss0 == literals[22] &&
            Test.ss0 == literals[23] &&
            Test.ss0 == literals[24]);

    test(Test.ss3 == "\\\\U\\u\\" &&
            Test.ss3 == literals[25]);

    test(Test.ss4 == "\\A\\" &&
            Test.ss4 == literals[26]);

    test(Test.ss5 == "\\u0041\\" &&
            Test.ss5 == literals[27]);

    test(Test.su0 == Test.su1 &&
            Test.su0 == Test.su2 &&
            Test.su0 == literals[28] &&
            Test.su0 == literals[29] &&
            Test.su0 == literals[30]);

    await prx.ice_ping();

    test(await prx.ice_isA(Test.MyClass.ice_staticId()));

    test((await prx.ice_id()) === Test.MyDerivedClass.ice_staticId());

    test((await prx.ice_ids()).length === 3);

    await prx.opVoid();

    {
        const [retval, p3] = await prx.opByte(0xff, 0x0f);
        test(p3 === 0xf0);
        test(retval === 0xff);
    }

    {
        const [retval, p3] = await prx.opBool(true, false);
        test(p3);
        test(!retval);
    }

    {
        const lo = new Ice.Long(0, 12);
        const [retval, s, i, l] = await prx.opShortIntLong(10, 11, lo);

        test(s === 10);
        test(i === 11);
        test(l.equals(lo));
        test(retval.equals(lo));
    }

    {
        const [retval, f, d] = await prx.opFloatDouble(3.14, 1.1E10);
        test((f - 3.14) <= 0.01);
        test(d == 1.1E10);
        test(retval == 1.1E10);
    }

    try
    {
        await prx.opByte(0xffff, 0xff0f);
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    try
    {
        await prx.opShortIntLong(-32768 - 1, 0, new Ice.Long(0));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    try
    {
        await prx.opShortIntLong(32767 + 1, 0, new Ice.Long(0));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    try
    {
        await prx.opShortIntLong(0, -2147483648 - 1, new Ice.Long(0));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    try
    {
        await prx.opShortIntLong(0, 2147483647 + 1, new Ice.Long(0));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    try
    {
        await prx.opShortIntLong(0, 0, new Ice.Long(0, 0xFFFFFFFF + 1));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof RangeError, ex);
    }

    try
    {
        await prx.opShortIntLong(0, 0, new Ice.Long(0xFFFFFFFF + 1, 0));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof RangeError, ex);
    }

    try
    {
        await prx.opShortIntLong(0, 0, new Ice.Long(0, -1));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof RangeError, ex);
    }

    try
    {
        await prx.opShortIntLong(Number.NaN, 0, new Ice.Long(0, 0));
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    try
    {
        await prx.opFloatDouble(Number.MAX_VALUE, 0);
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    try
    {
        await prx.opFloatDouble(-Number.MAX_VALUE, 0);
        test(false);
    }
    catch(ex)
    {
        test(ex instanceof Ice.MarshalException, ex);
    }

    await prx.opFloatDouble(Number.NaN, Number.NaN);
    await prx.opFloatDouble(-Number.NaN, -Number.NaN);
    await prx.opFloatDouble(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);
    await prx.opFloatDouble(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY);

    {
        const [retval, p3] = await prx.opString("hello", "world");
        test(p3 === "world hello");
        test(retval === "hello world");
    }

    {
        const [retval, p2] = await prx.opMyEnum(Test.MyEnum.enum2);
        test(p2 === Test.MyEnum.enum2);
        test(retval === Test.MyEnum.enum3);
    }

    {
        // Test null enum
        const [retval, p2] = await prx.opMyEnum(null);
        test(p2 === Test.MyEnum.enum1);
        test(retval === Test.MyEnum.enum3);
    }

    {
            const [retval, p2, p3] = await prx.opMyClass(prx);

        test(p2.ice_getIdentity().equals(Ice.stringToIdentity("test")));
        test(p3.ice_getIdentity().equals(Ice.stringToIdentity("noSuchIdentity")));
        test(retval.ice_getIdentity().equals(Ice.stringToIdentity("test")));
    }

    {
        const si1 = new Test.Structure();
        si1.p = prx;
        si1.e = Test.MyEnum.enum3;
        si1.s = new Test.AnotherStruct();
        si1.s.s = "abc";
        const si2 = new Test.Structure();
        si2.p = null;
        si2.e = Test.MyEnum.enum2;
        si2.s = new Test.AnotherStruct();
        si2.s.s = "def";

        const [retval, p3] = await prx.opStruct(si1, si2);
        test(retval.p === null);
        test(retval.e === Test.MyEnum.enum2);
        test(retval.s.s === "def");
        test(p3.p !== null);
        test(p3.e === Test.MyEnum.enum3);
        test(p3.s.s === "a new string");
    }

    {
        const si1 = new Test.Structure();
        // Test null struct
        const [retval, p3] = await prx.opStruct(si1, null);

        test(retval.p === null);
        test(retval.e === Test.MyEnum.enum1);
        test(retval.s.s === "");
        test(p3.p === null);
        test(p3.e === Test.MyEnum.enum1);
        test(p3.s.s === "a new string");
    }

    {
        const bsi1 = new Uint8Array([0x01, 0x11, 0x12, 0x22]);
        const bsi2 = new Uint8Array([0xf1, 0xf2, 0xf3, 0xf4]);

        const [retval, p3] = await prx.opByteS(bsi1, bsi2);

        test(p3.length === 4);
        test(p3[0] === 0x22);
        test(p3[1] === 0x12);
        test(p3[2] === 0x11);
        test(p3[3] === 0x01);
        test(retval.length === 8);
        test(retval[0] === 0x01);
        test(retval[1] === 0x11);
        test(retval[2] === 0x12);
        test(retval[3] === 0x22);
        test(retval[4] === 0xf1);
        test(retval[5] === 0xf2);
        test(retval[6] === 0xf3);
        test(retval[7] === 0xf4);
    }

    {
        const bsi1 = [true, true, false];
        const bsi2 = [false];
        const [retval, p3] = await prx.opBoolS(bsi1, bsi2);

        test(p3.length == 4);
        test(p3[0]);
        test(p3[1]);
        test(!p3[2]);
        test(!p3[3]);
        test(retval.length == 3);
        test(!retval[0]);
        test(retval[1]);
        test(retval[2]);
    }

    {
        const ssi = [1, 2, 3];
        const isi = [5, 6, 7, 8];
        const l1 = new Ice.Long(0, 10);
        const l2 = new Ice.Long(0, 30);
        const l3 = new Ice.Long(0, 20);
        const lsi = [l1, l2, l3];
        const [retval, sso, iso, lso] = await prx.opShortIntLongS(ssi, isi, lsi);

        test(sso.length === 3);
        test(sso[0] === 1);
        test(sso[1] === 2);
        test(sso[2] === 3);
        test(iso.length === 4);
        test(iso[0] === 8);
        test(iso[1] === 7);
        test(iso[2] === 6);
        test(iso[3] === 5);
        test(lso.length === 6);
        test(lso[0].equals(l1));
        test(lso[1].equals(l2));
        test(lso[2].equals(l3));
        test(lso[3].equals(l1));
        test(lso[4].equals(l2));
        test(lso[5].equals(l3));
        test(retval.length === 3);
        test(retval[0].equals(l1));
        test(retval[1].equals(l2));
        test(retval[2].equals(l3));
    }

    {
        const fsi = [3.14, 1.11];
        const dsi = [1.1E10, 1.2E10, 1.3E10];
        const [retval, fso, dso] = await prx.opFloatDoubleS(fsi, dsi);

        test(fso.length === 2);
        test((fso[0] - 3.14) <= 0.01);
        test((fso[1] - 1.11) <= 0.01);
        test(dso.length === 3);
        test(dso[0] === 1.3E10);
        test(dso[1] === 1.2E10);
        test(dso[2] === 1.1E10);
        test(retval.length === 5);
        test(retval[0] === 1.1E10);
        test(retval[1] === 1.2E10);
        test(retval[2] === 1.3E10);
        test((retval[3] - 3.14) <= 0.01);
        test((retval[4] - 1.11) <= 0.01);
    }

    {
        const ssi1 = ["abc", "de", "fghi"];
        const ssi2 = ["xyz"];
        const [retval, sso] = await prx.opStringS(ssi1, ssi2);

        test(sso.length === 4);
        test(sso[0] === "abc");
        test(sso[1] === "de");
        test(sso[2] === "fghi");
        test(sso[3] === "xyz");
        test(retval.length === 3);
        test(retval[0] === "fghi");
        test(retval[1] === "de");
        test(retval[2] === "abc");
    }

    {
        const bsi1 =
            [
                new Uint8Array([0x01, 0x11, 0x12]),
                new Uint8Array([0xff])
            ];

        const bsi2 =
            [
                new Uint8Array([0x0e]),
                new Uint8Array([0xf2, 0xf1])
            ];

        const [retval, bso] = await prx.opByteSS(bsi1, bsi2);

        test(bso.length === 2);
        test(bso[0].length === 1);
        test(bso[0][0] === 0xff);
        test(bso[1].length === 3);
        test(bso[1][0] === 0x01);
        test(bso[1][1] === 0x11);
        test(bso[1][2] === 0x12);
        test(retval.length === 4);
        test(retval[0].length === 3);
        test(retval[0][0] === 0x01);
        test(retval[0][1] === 0x11);
        test(retval[0][2] === 0x12);
        test(retval[1].length === 1);
        test(retval[1][0] === 0xff);
        test(retval[2].length === 1);
        test(retval[2][0] === 0x0e);
        test(retval[3].length === 2);
        test(retval[3][0] === 0xf2);
        test(retval[3][1] === 0xf1);
    }

    {
        const bsi1 =
            [
                [true],
                [false],
                [true, true]
            ];

        const bsi2 =
            [
                [false, false, true]
            ];

        const [retval, bso] = await prx.opBoolSS(bsi1, bsi2);

        test(bso.length === 4);
        test(bso[0].length === 1);
        test(bso[0][0]);
        test(bso[1].length === 1);
        test(!bso[1][0]);
        test(bso[2].length === 2);
        test(bso[2][0]);
        test(bso[2][1]);
        test(bso[3].length === 3);
        test(!bso[3][0]);
        test(!bso[3][1]);
        test(bso[3][2]);
        test(retval.length === 3);
        test(retval[0].length === 2);
        test(retval[0][0]);
        test(retval[0][1]);
        test(retval[1].length === 1);
        test(!retval[1][0]);
        test(retval[2].length === 1);
        test(retval[2][0]);
    }

    {
        const ssi =
            [
                [1, 2, 5],
                [13],
                []
            ];

        const isi =
            [
                [24, 98],
                [42]
            ];

        const l1 = new Ice.Long(0, 496);
        const l2 = new Ice.Long(0, 1729);

        const lsi =
            [
                [l1, l2]
            ];

        const [retval, sso, iso, lso] = await prx.opShortIntLongSS(ssi, isi, lsi);

        test(retval.length === 1);
        test(retval[0].length === 2);
        test(retval[0][0].equals(l1));
        test(retval[0][1].equals(l2));
        test(sso.length === 3);
        test(sso[0].length === 3);
        test(sso[0][0] === 1);
        test(sso[0][1] === 2);
        test(sso[0][2] === 5);
        test(sso[1].length === 1);
        test(sso[1][0] === 13);
        test(sso[2].length === 0);
        test(iso.length === 2);
        test(iso[0].length === 1);
        test(iso[0][0] === 42);
        test(iso[1].length === 2);
        test(iso[1][0] === 24);
        test(iso[1][1] === 98);
        test(lso.length === 2);
        test(lso[0].length === 2);
        test(lso[0][0].equals(l1));
        test(lso[0][1].equals(l2));
        test(lso[1].length === 2);
        test(lso[1][0].equals(l1));
        test(lso[1][1].equals(l2));
    }

    {
        const fsi =
            [
                [3.14],
                [1.11],
                []
            ];

        const dsi =
            [
                [1.1E10, 1.2E10, 1.3E10]
            ];

        const [retval, fso, dso] = await prx.opFloatDoubleSS(fsi, dsi);

        test(fso.length === 3);
        test(fso[0].length === 1);
        test((fso[0][0] - 3.14) <= 0.01);
        test(fso[1].length === 1);
        test((fso[1][0] - 1.11) <= 0.01);
        test(fso[2].length === 0);
        test(dso.length === 1);
        test(dso[0].length === 3);
        test(dso[0][0] === 1.1E10);
        test(dso[0][1] === 1.2E10);
        test(dso[0][2] === 1.3E10);
        test(retval.length === 2);
        test(retval[0].length === 3);
        test(retval[0][0] === 1.1E10);
        test(retval[0][1] === 1.2E10);
        test(retval[0][2] === 1.3E10);
        test(retval[1].length === 3);
        test(retval[1][0] === 1.1E10);
        test(retval[1][1] === 1.2E10);
        test(retval[1][2] === 1.3E10);
    }

    {
        const ssi1 =
            [
                ["abc"],
                ["de", "fghi"]
            ];

        const ssi2 =
            [
                [],
                [],
                ["xyz"]
            ];

        const [retval, sso] = await prx.opStringSS(ssi1, ssi2);
        test(sso.length === 5);
        test(sso[0].length === 1);
        test(sso[0][0] === "abc");
        test(sso[1].length === 2);
        test(sso[1][0] === "de");
        test(sso[1][1] === "fghi");
        test(sso[2].length === 0);
        test(sso[3].length === 0);
        test(sso[4].length === 1);
        test(sso[4][0] === "xyz");
        test(retval.length === 3);
        test(retval[0].length === 1);
        test(retval[0][0] === "xyz");
        test(retval[1].length === 0);
        test(retval[2].length === 0);
    }

    {
        const sssi1 =
            [
                [
                    ["abc", "de"],
                    ["xyz"]
                ],
                [
                    ["hello"]
                ]
            ];

        const sssi2 =
            [
                [
                    ["", ""],
                    ["abcd"]
                ],
                [
                    [""]
                ],
                []
            ];

        const [retval, ssso] = await prx.opStringSSS(sssi1, sssi2);

        test(ssso.length === 5);
        test(ssso[0].length === 2);
        test(ssso[0][0].length === 2);
        test(ssso[0][1].length === 1);
        test(ssso[1].length === 1);
        test(ssso[1][0].length === 1);
        test(ssso[2].length === 2);
        test(ssso[2][0].length === 2);
        test(ssso[2][1].length === 1);
        test(ssso[3].length === 1);
        test(ssso[3][0].length === 1);
        test(ssso[4].length === 0);
        test(ssso[0][0][0] === "abc");
        test(ssso[0][0][1] === "de");
        test(ssso[0][1][0] === "xyz");
        test(ssso[1][0][0] === "hello");
        test(ssso[2][0][0] === "");
        test(ssso[2][0][1] === "");
        test(ssso[2][1][0] === "abcd");
        test(ssso[3][0][0] === "");

        test(retval.length === 3);
        test(retval[0].length === 0);
        test(retval[1].length === 1);
        test(retval[1][0].length === 1);
        test(retval[2].length === 2);
        test(retval[2][0].length === 2);
        test(retval[2][1].length === 1);
        test(retval[1][0][0] === "");
        test(retval[2][0][0] === "");
        test(retval[2][0][1] === "");
        test(retval[2][1][0] === "abcd");
    }

    {
        const di1 = new Test.ByteBoolD();
        di1.set(10, true);
        di1.set(100, false);

        const di2 = new Test.ByteBoolD();
        di2.set(10, true);
        di2.set(11, false);
        di2.set(101, true);

        const [retval, p3] = await prx.opByteBoolD(di1, di2);
        test(Ice.MapUtil.equals(p3, di1));
        test(retval.size === 4);
        test(retval.get(10) === true);
        test(retval.get(11) === false);
        test(retval.get(100) === false);
        test(retval.get(101) === true);
    }

    {
        const di1 = new Test.ShortIntD();
        di1.set(110, -1);
        di1.set(1100, 123123);

        const di2 = new Test.ShortIntD();
        di2.set(110, -1);
        di2.set(111, -100);
        di2.set(1101, 0);

        const [retval, p3] = await prx.opShortIntD(di1, di2);
        test(Ice.MapUtil.equals(p3, di1));
        test(retval.size === 4);
        test(retval.get(110) === -1);
        test(retval.get(111) === -100);
        test(retval.get(1100) === 123123);
        test(retval.get(1101) === 0);
    }

    {
        const di1 = new Test.LongFloatD();
        di1.set(new Ice.Long(0, 999999110), -1.1);
        di1.set(new Ice.Long(0, 999999111), 123123.2);

        const di2 = new Test.LongFloatD();
        di2.set(new Ice.Long(0, 999999110), -1.1);
        di2.set(new Ice.Long(0, 999999120), -100.4);
        di2.set(new Ice.Long(0, 999999130), 0.5);

        const [retval, p3] = await prx.opLongFloatD(di1, di2);

        test(p3.equals(di1, (v1, v2) => (Math.abs(v1) - Math.abs(v2)) <= 0.01));
        test(retval.size === 4);
        test(Math.abs(retval.get(new Ice.Long(0, 999999110))) - Math.abs(-1.1) <= 0.01);
        test(Math.abs(retval.get(new Ice.Long(0, 999999120))) - Math.abs(-100.4) <= 0.01);
        test(retval.get(new Ice.Long(0, 999999111)) - 123123.2 <= 0.01);
        test(retval.get(new Ice.Long(0, 999999130)) - 0.5 <= 0.01);
    }

    {
        const di1 = new Test.StringStringD();
        di1.set("foo", "abc -1.1");
        di1.set("bar", "abc 123123.2");

        const di2 = new Test.StringStringD();
        di2.set("foo", "abc -1.1");
        di2.set("FOO", "abc -100.4");
        di2.set("BAR", "abc 0.5");

        const [retval, p3] = await prx.opStringStringD(di1, di2);
        test(Ice.MapUtil.equals(p3, di1));
        test(retval.size == 4);
        test(retval.get("foo") === "abc -1.1");
        test(retval.get("FOO") === "abc -100.4");
        test(retval.get("bar") === "abc 123123.2");
        test(retval.get("BAR") === "abc 0.5");
    }

    {
        const di1 = new Test.StringMyEnumD();
        di1.set("abc", Test.MyEnum.enum1);
        di1.set("", Test.MyEnum.enum2);

        const di2 = new Test.StringMyEnumD();
        di2.set("abc", Test.MyEnum.enum1);
        di2.set("qwerty", Test.MyEnum.enum3);
        di2.set("Hello!!", Test.MyEnum.enum2);

        const [retval, p3] = await prx.opStringMyEnumD(di1, di2);

        test(Ice.MapUtil.equals(p3, di1));
        test(retval.size === 4);
        test(retval.get("abc") === Test.MyEnum.enum1);
        test(retval.get("qwerty") === Test.MyEnum.enum3);
        test(retval.get("") === Test.MyEnum.enum2);
        test(retval.get("Hello!!") === Test.MyEnum.enum2);
    }

    {
        const di1 = new Test.MyEnumStringD();
        di1.set(Test.MyEnum.enum1, "abc");

        const di2 = new Test.MyEnumStringD();
        di2.set(Test.MyEnum.enum2, "Hello!!");
        di2.set(Test.MyEnum.enum3, "qwerty");

        const [retval, p3] = await prx.opMyEnumStringD(di1, di2);

        test(Ice.MapUtil.equals(p3, di1));
        test(retval.size === 3);
        test(retval.get(Test.MyEnum.enum1) === "abc");
        test(retval.get(Test.MyEnum.enum2) === "Hello!!");
        test(retval.get(Test.MyEnum.enum3) === "qwerty");
    }

    {
        const s11 = new Test.MyStruct(1, 1);
        const s12 = new Test.MyStruct(1, 2);
        const di1 = new Test.MyStructMyEnumD();
        di1.set(s11, Test.MyEnum.enum1);
        di1.set(s12, Test.MyEnum.enum2);
        const s22 = new Test.MyStruct(2, 2);
        const s23 = new Test.MyStruct(2, 3);
        const di2 = new Test.MyStructMyEnumD();
        di2.set(s11, Test.MyEnum.enum1);
        di2.set(s22, Test.MyEnum.enum3);
        di2.set(s23, Test.MyEnum.enum2);

        const [retval, p3] = await prx.opMyStructMyEnumD(di1, di2);
        test(p3.equals(di1));

        test(retval.size === 4);
        test(retval.get(s11) === Test.MyEnum.enum1);
        test(retval.get(s12) === Test.MyEnum.enum2);
        test(retval.get(s22) === Test.MyEnum.enum3);
        test(retval.get(s23) === Test.MyEnum.enum2);
    }

    {
        const ds1 = new Test.ByteBoolD();
        ds1.set(10, true);
        ds1.set(100, false);
        const ds2 = new Test.ByteBoolD();
        ds2.set(10, true);
        ds2.set(11, false);
        ds2.set(101, true);
        const ds3 = new Test.ByteBoolD();
        ds3.set(100, false);
        ds3.set(101, false);

        const [retval, p3] = await prx.opByteBoolDS([ds1, ds2], [ds3]);
        test(retval.length == 2);
        test(retval[0].size == 3);
        test(retval[0].get(10) === true);
        test(retval[0].get(11) === false);
        test(retval[0].get(101) === true);
        test(retval[1].size === 2);
        test(retval[1].get(10) === true);
        test(retval[1].get(100) === false);
        test(p3.length == 3);
        test(p3[0].size == 2);
        test(p3[0].get(100) === false);
        test(p3[0].get(101) === false);
        test(p3[1].size == 2);
        test(p3[1].get(10) === true);
        test(p3[1].get(100) === false);
        test(p3[2].size == 3);
        test(p3[2].get(10) === true);
        test(p3[2].get(11) === false);
        test(p3[2].get(101) === true);
    }

    {
        const di1 = new Test.ShortIntD();
        di1.set(110, -1);
        di1.set(1100, 123123);
        const di2 = new Test.ShortIntD();
        di2.set(110, -1);
        di2.set(111, -100);
        di2.set(1101, 0);
        const di3 = new Test.ShortIntD();
        di3.set(100, -1001);

        const [retval, p3] = await prx.opShortIntDS([di1, di2], [di3]);
        test(retval.length == 2);
        test(retval[0].size == 3);
        test(retval[0].get(110) === -1);
        test(retval[0].get(111) === -100);
        test(retval[0].get(1101) === 0);
        test(retval[1].size === 2);
        test(retval[1].get(110) === -1);
        test(retval[1].get(1100) === 123123);

        test(p3.length === 3);
        test(p3[0].size === 1);
        test(p3[0].get(100) === -1001);
        test(p3[1].size === 2);
        test(p3[1].get(110) === -1);
        test(p3[1].get(1100) === 123123);
        test(p3[2].size === 3);
        test(p3[2].get(110) === -1);
        test(p3[2].get(111) === -100);
        test(p3[2].get(1101) === 0);
    }

    {
        const di1 = new Test.LongFloatD();
        di1.set(new Ice.Long(0, 999999110), -1.1);
        di1.set(new Ice.Long(0, 999999111), 123123.2);
        const di2 = new Test.LongFloatD();
        di2.set(new Ice.Long(0, 999999110), -1.1);
        di2.set(new Ice.Long(0, 999999120), -100.4);
        di2.set(new Ice.Long(0, 999999130), 0.5);
        const di3 = new Test.LongFloatD();
        di3.set(new Ice.Long(0, 999999140), 3.14);

        const [retval, p3] = await prx.opLongFloatDS([di1, di2], [di3]);
        test(retval.length == 2);
        test(retval[0].size == 3);
        test(retval[0].get(new Ice.Long(0, 999999110)) - Math.abs(-1.1) <= 0.1);
        test(retval[0].get(new Ice.Long(0, 999999120)) - Math.abs(-100.4) <= 0.1);
        test(retval[0].get(new Ice.Long(0, 999999130)) - 0.5 <= 0.1);
        test(retval[1].size == 2);
        test(retval[1].get(new Ice.Long(0, 999999110)) - Math.abs(-1.1) <= 0.1);
        test(retval[1].get(new Ice.Long(0, 999999111)) - 123123.2 <= 0.1);

        test(p3.length == 3);
        test(p3[0].size == 1);
        test(p3[0].get(new Ice.Long(0, 999999140)) - 3.14 <= 0.1);
        test(p3[1].size == 2);
        test(p3[1].get(new Ice.Long(0, 999999110)) - Math.abs(-1.1) <= 0.1);
        test(p3[1].get(new Ice.Long(0, 999999111)) - 123123.2 <= 0.1);
        test(p3[2].size == 3);
        test(p3[2].get(new Ice.Long(0, 999999110)) - Math.abs(-1.1) <= 0.1);
        test(p3[2].get(new Ice.Long(0, 999999120)) - Math.abs(-100.4) <= 0.1);
        test(p3[2].get(new Ice.Long(0, 999999130)) - 0.5 <= 0.1);
    }

    {
        const di1 = new Test.StringStringD();
        di1.set("foo", "abc -1.1");
        di1.set("bar", "abc 123123.2");
        const di2 = new Test.StringStringD();
        di2.set("foo", "abc -1.1");
        di2.set("FOO", "abc -100.4");
        di2.set("BAR", "abc 0.5");
        const di3 = new Test.StringStringD();
        di3.set("f00", "ABC -3.14");

        const [retval, p3] = await prx.opStringStringDS([di1, di2], [di3]);
        test(retval.length === 2);
        test(retval[0].size === 3);
        test(retval[0].get("foo") === "abc -1.1");
        test(retval[0].get("FOO") === "abc -100.4");
        test(retval[0].get("BAR") === "abc 0.5");
        test(retval[1].size === 2);
        test(retval[1].get("foo") === "abc -1.1");
        test(retval[1].get("bar") === "abc 123123.2");

        test(p3.length === 3);
        test(p3[0].size === 1);
        test(p3[0].get("f00") === "ABC -3.14");
        test(p3[1].size === 2);
        test(p3[1].get("foo") === "abc -1.1");
        test(p3[1].get("bar") === "abc 123123.2");
        test(p3[2].size === 3);
        test(p3[2].get("foo") === "abc -1.1");
        test(p3[2].get("FOO") === "abc -100.4");
        test(p3[2].get("BAR") === "abc 0.5");
    }

    {
        const di1 = new Test.StringMyEnumD();
        di1.set("abc", Test.MyEnum.enum1);
        di1.set("", Test.MyEnum.enum2);
        const di2 = new Test.StringMyEnumD();
        di2.set("abc", Test.MyEnum.enum1);
        di2.set("qwerty", Test.MyEnum.enum3);
        di2.set("Hello!!", Test.MyEnum.enum2);
        const di3 = new Test.StringMyEnumD();
        di3.set("Goodbye", Test.MyEnum.enum1);

        const [retval, p3] = await prx.opStringMyEnumDS([di1, di2], [di3]);

        test(retval.length == 2);
        test(retval[0].size == 3);
        test(retval[0].get("abc") == Test.MyEnum.enum1);
        test(retval[0].get("qwerty") == Test.MyEnum.enum3);
        test(retval[0].get("Hello!!") == Test.MyEnum.enum2);
        test(retval[1].size == 2);
        test(retval[1].get("abc") == Test.MyEnum.enum1);
        test(retval[1].get("") == Test.MyEnum.enum2);

        test(p3.length == 3);
        test(p3[0].size == 1);
        test(p3[0].get("Goodbye") == Test.MyEnum.enum1);
        test(p3[1].size == 2);
        test(p3[1].get("abc") == Test.MyEnum.enum1);
        test(p3[1].get("") == Test.MyEnum.enum2);
        test(p3[2].size == 3);
        test(p3[2].get("abc") == Test.MyEnum.enum1);
        test(p3[2].get("qwerty") == Test.MyEnum.enum3);
        test(p3[2].get("Hello!!") == Test.MyEnum.enum2);
    }

    {
        const di1 = new Test.MyEnumStringD();
        di1.set(Test.MyEnum.enum1, "abc");
        const di2 = new Test.MyEnumStringD();
        di2.set(Test.MyEnum.enum2, "Hello!!");
        di2.set(Test.MyEnum.enum3, "qwerty");
        const di3 = new Test.MyEnumStringD();
        di3.set(Test.MyEnum.enum1, "Goodbye");

        const [retval, p3] = await prx.opMyEnumStringDS([di1, di2], [di3]);
        test(retval.length == 2);
        test(retval[0].size == 2);
        test(retval[0].get(Test.MyEnum.enum2) === "Hello!!");
        test(retval[0].get(Test.MyEnum.enum3) === "qwerty");
        test(retval[1].size == 1);
        test(retval[1].get(Test.MyEnum.enum1) === "abc");

        test(p3.length == 3);
        test(p3[0].size == 1);
        test(p3[0].get(Test.MyEnum.enum1) === "Goodbye");
        test(p3[1].size == 1);
        test(p3[1].get(Test.MyEnum.enum1) === "abc");
        test(p3[2].size == 2);
        test(p3[2].get(Test.MyEnum.enum2) === "Hello!!");
        test(p3[2].get(Test.MyEnum.enum3) === "qwerty");
    }

    {
        const s11 = new Test.MyStruct(1, 1);
        const s12 = new Test.MyStruct(1, 2);
        const di1 = new Test.MyStructMyEnumD();
        di1.set(s11, Test.MyEnum.enum1);
        di1.set(s12, Test.MyEnum.enum2);

        const s22 = new Test.MyStruct(2, 2);
        const s23 = new Test.MyStruct(2, 3);
        const di2 = new Test.MyStructMyEnumD();
        di2.set(s11, Test.MyEnum.enum1);
        di2.set(s22, Test.MyEnum.enum3);
        di2.set(s23, Test.MyEnum.enum2);

        const di3 = new Test.MyStructMyEnumD();
        di3.set(s23, Test.MyEnum.enum3);

        const [retval, p3] = await prx.opMyStructMyEnumDS([di1, di2], [di3]);
        test(retval.length == 2);
        test(retval[0].size == 3);
        test(retval[0].get(s11) === Test.MyEnum.enum1);
        test(retval[0].get(s22) === Test.MyEnum.enum3);
        test(retval[0].get(s23) === Test.MyEnum.enum2);
        test(retval[1].size == 2);
        test(retval[1].get(s11) === Test.MyEnum.enum1);
        test(retval[1].get(s12) === Test.MyEnum.enum2);

        test(p3.length == 3);
        test(p3[0].size == 1);
        test(p3[0].get(s23) === Test.MyEnum.enum3);
        test(p3[1].size == 2);
        test(p3[1].get(s11) === Test.MyEnum.enum1);
        test(p3[1].get(s12) === Test.MyEnum.enum2);
        test(p3[2].size == 3);
        test(p3[2].get(s11) === Test.MyEnum.enum1);
        test(p3[2].get(s22) === Test.MyEnum.enum3);
        test(p3[2].get(s23) === Test.MyEnum.enum2);
    }

    {
        const sdi1 = new Test.ByteByteSD();
        sdi1.set(0x01, new Uint8Array([0x01, 0x11]));
        sdi1.set(0x22, new Uint8Array([0x12]));
        const sdi2 = new Test.ByteByteSD();
        sdi2.set(0xf1, new Uint8Array([0xf2, 0xf3]));

        const [retval, p3] = await prx.opByteByteSD(sdi1, sdi2);
        test(p3.size == 1);
        test(p3.get(0xf1).length === 2);
        test(p3.get(0xf1)[0] === 0xf2);
        test(p3.get(0xf1)[1] === 0xf3);
        test(retval.size === 3);
        test(retval.get(0x01).length === 2);
        test(retval.get(0x01)[0] === 0x01);
        test(retval.get(0x01)[1] === 0x11);
        test(retval.get(0x22).length === 1);
        test(retval.get(0x22)[0] === 0x12);
        test(retval.get(0xf1).length === 2);
        test(retval.get(0xf1)[0] === 0xf2);
        test(retval.get(0xf1)[1] === 0xf3);
    }

    {
        const si1 = [true, false];
        const si2 = [false, true, true];
        const sdi1 = new Test.BoolBoolSD();
        sdi1.set(false, si1);
        sdi1.set(true, si2);
        const sdi2 = new Test.BoolBoolSD();
        sdi2.set(false, si1);

        const [retval, p3] = await prx.opBoolBoolSD(sdi1, sdi2);
        test(p3.size === 1);
        test(p3.get(false).length === 2);
        test(p3.get(false)[0] === true);
        test(p3.get(false)[1] === false);
        test(retval.size === 2);
        test(retval.get(false).length === 2);
        test(retval.get(false)[0] === true);
        test(retval.get(false)[1] === false);
        test(retval.get(true).length === 3);
        test(retval.get(true)[0] === false);
        test(retval.get(true)[1] === true);
        test(retval.get(true)[2] === true);
    }

    {
        const sdi1 = new Test.ShortShortSD();
        const sdi2 = new Test.ShortShortSD();

        const si1 = [1, 2, 3];
        const si2 = [4, 5];
        const si3 = [6, 7];

        sdi1.set(1, si1);
        sdi1.set(2, si2);
        sdi2.set(4, si3);

        const [retval, p3] = await prx.opShortShortSD(sdi1, sdi2);
        test(p3.size === 1);
        test(p3.get(4).length === 2);
        test(p3.get(4)[0] === 6);
        test(p3.get(4)[1] === 7);
        test(retval.size === 3);
        test(retval.get(1).length === 3);
        test(retval.get(1)[0] === 1);
        test(retval.get(1)[1] === 2);
        test(retval.get(1)[2] === 3);
        test(retval.get(2).length === 2);
        test(retval.get(2)[0] === 4);
        test(retval.get(2)[1] === 5);
        test(retval.get(4).length === 2);
        test(retval.get(4)[0] === 6);
        test(retval.get(4)[1] === 7);
    }

    {
        const sdi1 = new Test.IntIntSD();
        const sdi2 = new Test.IntIntSD();

        const si1 = [100, 200, 300];
        const si2 = [400, 500];
        const si3 = [600, 700];

        sdi1.set(100, si1);
        sdi1.set(200, si2);
        sdi2.set(400, si3);

        const [retval, p3] = await prx.opIntIntSD(sdi1, sdi2);
        test(p3.size === 1);
        test(p3.get(400).length === 2);
        test(p3.get(400)[0] === 600);
        test(p3.get(400)[1] === 700);
        test(retval.size === 3);
        test(retval.get(100).length === 3);
        test(retval.get(100)[0] === 100);
        test(retval.get(100)[1] === 200);
        test(retval.get(100)[2] === 300);
        test(retval.get(200).length === 2);
        test(retval.get(200)[0] === 400);
        test(retval.get(200)[1] === 500);
        test(retval.get(400).length === 2);
        test(retval.get(400)[0] === 600);
        test(retval.get(400)[1] === 700);
    }

    {
        const sdi1 = new Test.LongLongSD();
        const sdi2 = new Test.LongLongSD();

        const si1 = [new Ice.Long(0, 999999110), new Ice.Long(0, 999999111), new Ice.Long(0, 999999110)];
        const si2 = [new Ice.Long(0, 999999120), new Ice.Long(0, 999999130)];
        const si3 = [new Ice.Long(0, 999999110), new Ice.Long(0, 999999120)];

        sdi1.set(new Ice.Long(0, 999999990), si1);
        sdi1.set(new Ice.Long(0, 999999991), si2);
        sdi2.set(new Ice.Long(0, 999999992), si3);

        const [retval, p3] = await prx.opLongLongSD(sdi1, sdi2);
        test(p3.size == 1);
        test(p3.get(new Ice.Long(0, 999999992)).length === 2);
        test(p3.get(new Ice.Long(0, 999999992))[0].equals(new Ice.Long(0, 999999110)));
        test(p3.get(new Ice.Long(0, 999999992))[1].equals(new Ice.Long(0, 999999120)));
        test(retval.size == 3);
        test(retval.get(new Ice.Long(0, 999999990)).length === 3);
        test(retval.get(new Ice.Long(0, 999999990))[0].equals(new Ice.Long(0, 999999110)));
        test(retval.get(new Ice.Long(0, 999999990))[1].equals(new Ice.Long(0, 999999111)));
        test(retval.get(new Ice.Long(0, 999999990))[2].equals(new Ice.Long(0, 999999110)));
        test(retval.get(new Ice.Long(0, 999999991)).length === 2);
        test(retval.get(new Ice.Long(0, 999999991))[0].equals(new Ice.Long(0, 999999120)));
        test(retval.get(new Ice.Long(0, 999999991))[1].equals(new Ice.Long(0, 999999130)));
        test(retval.get(new Ice.Long(0, 999999992)).length === 2);
        test(retval.get(new Ice.Long(0, 999999992))[0].equals(new Ice.Long(0, 999999110)));
        test(retval.get(new Ice.Long(0, 999999992))[1].equals(new Ice.Long(0, 999999120)));
    }

    {
        const sdi1 = new Test.StringFloatSD();
        const sdi2 = new Test.StringFloatSD();

        const si1 = [-1.1, 123123.2, 100.0];
        const si2 = [42.24, -1.61];
        const si3 = [-3.14, 3.14];

        sdi1.set("abc", si1);
        sdi1.set("ABC", si2);
        sdi2.set("aBc", si3);

        const [retval, p3] = await prx.opStringFloatSD(sdi1, sdi2);

        test(p3.size === 1);
        test(p3.get("aBc").length === 2);
        test(p3.get("aBc")[0] - Math.abs(3.14) <= 0.1);
        test(p3.get("aBc")[1] - 3.14 <= 0.1);
        test(retval.size === 3);
        test(retval.get("abc").length === 3);
        test(retval.get("abc")[0] - Math.abs(1.1) <= 0.1);
        test(retval.get("abc")[1] - 123123.2 <= 0.1);
        test(retval.get("abc")[2] - 100.0 <= 0.1);
        test(retval.get("ABC").length === 2);
        test(retval.get("ABC")[0] - 42.24 <= 0.1);
        test(retval.get("ABC")[1] - Math.abs(1.61) <= 0.1);
        test(retval.get("aBc").length === 2);
        test(retval.get("aBc")[0] - Math.abs(3.14) <= 0.1);
        test(retval.get("aBc")[1] - 3.14 <= 0.1);
    }

    {
        const sdi1 = new Test.StringDoubleSD();
        const sdi2 = new Test.StringDoubleSD();

        const si1 = [1.1E10, 1.2E10, 1.3E10];
        const si2 = [1.4E10, 1.5E10];
        const si3 = [1.6E10, 1.7E10];

        sdi1.set("Hello!!", si1);
        sdi1.set("Goodbye", si2);
        sdi2.set("", si3);

        const [retval, p3] = await prx.opStringDoubleSD(sdi1, sdi2);

        test(p3.size === 1);
        test(p3.get("").length === 2);
        test(p3.get("")[0] === 1.6E10);
        test(p3.get("")[1] === 1.7E10);
        test(retval.size === 3);
        test(retval.get("Hello!!").length === 3);
        test(retval.get("Hello!!")[0] === 1.1E10);
        test(retval.get("Hello!!")[1] === 1.2E10);
        test(retval.get("Hello!!")[2] === 1.3E10);
        test(retval.get("Goodbye").length === 2);
        test(retval.get("Goodbye")[0] === 1.4E10);
        test(retval.get("Goodbye")[1] === 1.5E10);
        test(retval.get("").length === 2);
        test(retval.get("")[0] === 1.6E10);
        test(retval.get("")[1] === 1.7E10);
    }

    {
        const sdi1 = new Test.StringStringSD();
        const sdi2 = new Test.StringStringSD();

        const si1 = ["abc", "de", "fghi"];
        const si2 = ["xyz", "or"];
        const si3 = ["and", "xor"];

        sdi1.set("abc", si1);
        sdi1.set("def", si2);
        sdi2.set("ghi", si3);

        const [retval, p3] = await prx.opStringStringSD(sdi1, sdi2);

        test(p3.size === 1);
        test(p3.get("ghi").length === 2);
        test(p3.get("ghi")[0] === "and");
        test(p3.get("ghi")[1] === "xor");
        test(retval.size === 3);
        test(retval.get("abc").length === 3);
        test(retval.get("abc")[0] === "abc");
        test(retval.get("abc")[1] === "de");
        test(retval.get("abc")[2] === "fghi");
        test(retval.get("def").length === 2);
        test(retval.get("def")[0] === "xyz");
        test(retval.get("def")[1] === "or");
        test(retval.get("ghi").length === 2);
        test(retval.get("ghi")[0] === "and");
        test(retval.get("ghi")[1] === "xor");
    }

    {
        const sdi1 = new Test.MyEnumMyEnumSD();
        const sdi2 = new Test.MyEnumMyEnumSD();

        const si1 = [Test.MyEnum.enum1, Test.MyEnum.enum1, Test.MyEnum.enum2];
        const si2 = [Test.MyEnum.enum1, Test.MyEnum.enum2];
        const si3 = [Test.MyEnum.enum3, Test.MyEnum.enum3];

        sdi1.set(Test.MyEnum.enum3, si1);
        sdi1.set(Test.MyEnum.enum2, si2);
        sdi2.set(Test.MyEnum.enum1, si3);

        const [retval, p3] = await prx.opMyEnumMyEnumSD(sdi1, sdi2);
        test(p3.size == 1);
        test(p3.get(Test.MyEnum.enum1).length == 2);
        test(p3.get(Test.MyEnum.enum1)[0] == Test.MyEnum.enum3);
        test(p3.get(Test.MyEnum.enum1)[1] == Test.MyEnum.enum3);
        test(retval.size === 3);
        test(retval.get(Test.MyEnum.enum3).length == 3);
        test(retval.get(Test.MyEnum.enum3)[0] == Test.MyEnum.enum1);
        test(retval.get(Test.MyEnum.enum3)[1] == Test.MyEnum.enum1);
        test(retval.get(Test.MyEnum.enum3)[2] == Test.MyEnum.enum2);
        test(retval.get(Test.MyEnum.enum2).length == 2);
        test(retval.get(Test.MyEnum.enum2)[0] == Test.MyEnum.enum1);
        test(retval.get(Test.MyEnum.enum2)[1] == Test.MyEnum.enum2);
        test(retval.get(Test.MyEnum.enum1).length == 2);
        test(retval.get(Test.MyEnum.enum1)[0] == Test.MyEnum.enum3);
        test(retval.get(Test.MyEnum.enum1)[1] == Test.MyEnum.enum3);
    }

    {
        const lengths = [0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000];

        for(const l of lengths)
        {
            const s = new Array(l);
            for(let i = 0; i < l; ++i)
            {
                s[i] = i;
            }

            const r = await prx.opIntS(s);
            test(r.length == l);
            for(let j = 0; j < r.length; ++j)
            {
                test(r[j] == -j);
            }
        }
    }

    {
        const ctx = new Ice.Context();
        ctx.set("one", "ONE");
        ctx.set("two", "TWO");
        ctx.set("three", "THREE");
        {
            test(prx.ice_getContext().size === 0);
            const r = await prx.opContext();
            test(!Ice.MapUtil.equals(r, ctx));
        }

        {
            const r = await prx.opContext(ctx);
            test(prx.ice_getContext().size === 0);
            test(Ice.MapUtil.equals(r, ctx));
        }

        {
            const p2 = await Test.MyClassPrx.checkedCast(prx.ice_context(ctx));
            test(Ice.MapUtil.equals(p2.ice_getContext(), ctx));
            let r = await p2.opContext();
            test(Ice.MapUtil.equals(r, ctx));
            r = await p2.opContext(ctx);
            test(Ice.MapUtil.equals(r, ctx));
        }
    }

    if(!bidir)
    {
        //
        // Test implicit context propagation
        //
        const initData = new Ice.InitializationData();
        initData.properties = communicator.getProperties().clone();
        initData.properties.setProperty("Ice.ImplicitContext", "Shared");

        const ic = Ice.initialize(initData);

        let ctx = new Ice.Context();
        ctx.set("one", "ONE");
        ctx.set("two", "TWO");
        ctx.set("three", "THREE");

        let p3 = Test.MyClassPrx.uncheckedCast(ic.stringToProxy("test:" + helper.getTestEndpoint()));

        ic.getImplicitContext().setContext(ctx);
        test(Ice.MapUtil.equals(ic.getImplicitContext().getContext(), ctx));
        test(Ice.MapUtil.equals(await p3.opContext(), ctx));

        test(ic.getImplicitContext().containsKey("zero") == false);
        const r = ic.getImplicitContext().put("zero", "ZERO");
        test(r === undefined);
        test(ic.getImplicitContext().get("zero") == "ZERO");

        ctx = ic.getImplicitContext().getContext();
        test(Ice.MapUtil.equals(await p3.opContext(), ctx));

        const prxContext = new Ice.Context();
        prxContext.set("one", "UN");
        prxContext.set("four", "QUATRE");

        const combined = new Ice.Context(prxContext);
        for(const [key, value] of ctx)
        {
            if(!combined.has(key))
            {
                combined.set(key, value);
            }
        }

        test(combined.get("one") == "UN");

        p3 = Test.MyClassPrx.uncheckedCast(p3.ice_context(prxContext));

        ic.getImplicitContext().setContext(null);
        test(Ice.MapUtil.equals(await p3.opContext(), prxContext));

        ic.getImplicitContext().setContext(ctx);
        test(Ice.MapUtil.equals(await p3.opContext(), combined));

        test(ic.getImplicitContext().remove("one") == "ONE");

        await ic.destroy();
    }

    {
        const d = 1278312346.0 / 13.0;
        const ds = [];
        for(let i = 0; i < 5; i++)
        {
            ds[i] = d;
        }

        await prx.opDoubleMarshaling(d, ds);
        await prx.opIdempotent();
        await prx.opNonmutating();
    }

    {

        test(await prx.opByte1(0xFF) == 0xFF);
        test(await prx.opShort1(0x7FFF) == 0x7FFF);
        test(await prx.opInt1(0x7FFFFFFF) == 0x7FFFFFFF);
        test((await prx.opLong1(new Ice.Long(0x7FFFFFFF, 0xFFFFFFFF))).equals(new Ice.Long(0x7FFFFFFF, 0xFFFFFFFF)));
        test(await prx.opFloat1(1.0) == 1.0);
        test(await prx.opDouble1(1.0) == 1.0);
        test(await prx.opString1("opString1") == "opString1");
        test((await prx.opStringS1(null)).length === 0);
        test((await prx.opByteBoolD1(null)).size === 0);
        test((await prx.opStringS2(null)).length === 0);
        test((await prx.opByteBoolD2(null)).size === 0);

        const d = Test.MyDerivedClassPrx.uncheckedCast(prx);
        let s = new Test.MyStruct1();
        s.tesT = "Test.MyStruct1.s";
        s.myClass = null;
        s.myStruct1 = "Test.MyStruct1.myStruct1";
        s = await d.opMyStruct1(s);
        test(s.tesT == "Test.MyStruct1.s");
        test(s.myClass === null);
        test(s.myStruct1 == "Test.MyStruct1.myStruct1");
        let c = new Test.MyClass1();
        c.tesT = "Test.MyClass1.testT";
        c.myClass = null;
        c.myClass1 = "Test.MyClass1.myClass1";
        c = await d.opMyClass1(c);
        test(c.tesT == "Test.MyClass1.testT");
        test(c.myClass === null);
        test(c.myClass1 == "Test.MyClass1.myClass1");
    }

    {
        const p1 = await prx.opMStruct1();
        p1.e = Test.MyEnum.enum3;
        const [p2, p3] = await prx.opMStruct2(p1);
        test(p2.equals(p1) && p3.equals(p1));
    }

    {
        await prx.opMSeq1();
        const p1 = ["test"];
        const [p2, p3] = await prx.opMSeq2(p1);
        test(Ice.ArrayUtil.equals(p2, p1) && Ice.ArrayUtil.equals(p3, p1));
    }

    {
        await prx.opMDict1();
        const p1 = new Map();
        p1.set("test", "test");
        const [p2, p3] = await prx.opMDict2(p1);
        test(Ice.MapUtil.equals(p2, p1) && Ice.MapUtil.equals(p3, p1));
    }

    {
        const ds = [];
        for(let i = 0; i < 5; i++)
        {
            ds[i] = 1278312346.0 / 13.0;
        }
        await prx.opDoubleMarshaling(1278312346.0 / 13.0, ds);
    }
}
