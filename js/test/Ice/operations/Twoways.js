// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var run = function(communicator, prx, Test, bidir)
    {
        var p = new Ice.Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        var failCB = function(){ test(false); };

        var si1, si2, ctx, combined;
        Ice.Promise.try(() =>
            {
                ctx = new Ice.Context();
                ctx.set("one", "ONE");
                ctx.set("two", "TWO");
                ctx.set("three", "THREE");

                return prx.opStringLiterals();
            }
        ).then(literals =>
            {
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

                test(Test.ss0 == "\'\"\?\\\u0007\b\f\n\r\t\v" &&
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

                return prx.ice_ping();
            }
        ).then(() => prx.ice_isA(Test.MyClass.ice_staticId())
        ).then(b =>
            {
                test(b);
                return prx.ice_id();
            }
        ).then(id =>
            {
                test(id === Test.MyDerivedClass.ice_staticId());
                return prx.ice_ids();
            }
        ).then(ids =>
            {
                test(ids.length === 3);
                return prx.opVoid();
            }
        ).then(() => prx.opByte(0xff, 0x0f)
        ).then(r =>
            {
                var [retval, p3] = r;
                test(p3 === 0xf0);
                test(retval === 0xff);
                return prx.opBool(true, false);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(p3);
                test(!retval);
                var lo = new Ice.Long(0, 12);
                return prx.opShortIntLong(10, 11, lo);
            }
        ).then(r =>
            {
                var [retval, s, i, l] = r;
                var lo = new Ice.Long(0, 12);
                test(s === 10);
                test(i === 11);
                test(l.equals(lo));
                test(retval.equals(lo));
                return prx.opFloatDouble(3.14, 1.1E10);
            }
       ).then(r =>
            {
                var [retval, f, d] = r;
                test((f - 3.14) <= 0.01);
                test(d == 1.1E10);
                test(retval == 1.1E10);
                return prx.opByte(0xffff, 0xff0f);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opShortIntLong(-32768 - 1, 0, 0);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opShortIntLong(32767 + 1, 0, 0);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opShortIntLong(0, -2147483648 - 1, 0);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opShortIntLong(0, 2147483647 + 1, 0);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opShortIntLong(0, 0, new Ice.Long(0, 0xFFFFFFFF + 1));
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof RangeError);
                return prx.opShortIntLong(0, 0, new Ice.Long(0xFFFFFFFF + 1, 0));
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof RangeError);
                return prx.opShortIntLong(0, 0, new Ice.Long(0, -1));
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof RangeError);
                return prx.opShortIntLong(Number.NaN, 0, new Ice.Long(0, 0));
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opFloatDouble(Number.MAX_VALUE, 0);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opFloatDouble(-Number.MAX_VALUE, 0);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.MarshalException);
                return prx.opFloatDouble(Number.NaN, Number.NaN);
            }
        ).then(r =>
            {
                var [retval, f, d] = r;
                return prx.opFloatDouble(-Number.NaN, -Number.NaN);
            }
        ).then(r =>
            {
                var [retval, f, d] = r;
                return prx.opFloatDouble(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);
            }
        ).then(r =>
            {
                var [retval, f, d] = r;
                return prx.opFloatDouble(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY);
            }
        ).then(r =>
            {
                var [retval, f, d] = r;
                return prx.opString("hello", "world");
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(p3 === "world hello");
                test(retval === "hello world");
                return prx.opMyEnum(Test.MyEnum.enum2);
            }
        ).then(r =>
            {
                var [retval, p2] = r;
                test(p2 === Test.MyEnum.enum2);
                test(retval === Test.MyEnum.enum3);
                // Test null enum
                return prx.opMyEnum(null);
            }
        ).then(r =>
            {
                var [retval, p2] = r;
                test(p2 === Test.MyEnum.enum1);
                test(retval === Test.MyEnum.enum3);
                return prx.opMyClass(prx);
            }
        ).then(r =>
            {
                var [retval, p2, p3] = r;
                test(p2.ice_getIdentity().equals(Ice.stringToIdentity("test")));
                test(p3.ice_getIdentity().equals(Ice.stringToIdentity("noSuchIdentity")));
                test(retval.ice_getIdentity().equals(Ice.stringToIdentity("test")));

                si1 = new Test.Structure();
                si1.p = prx;
                si1.e = Test.MyEnum.enum3;
                si1.s = new Test.AnotherStruct();
                si1.s.s = "abc";
                si2 = new Test.Structure();
                si2.p = null;
                si2.e = Test.MyEnum.enum2;
                si2.s = new Test.AnotherStruct();
                si2.s.s = "def";
                return prx.opStruct(si1, si2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(retval.p === null);
                test(retval.e === Test.MyEnum.enum2);
                test(retval.s.s === "def");
                test(p3.p !== null);
                test(p3.e === Test.MyEnum.enum3);
                test(p3.s.s === "a new string");

                si1 = new Test.Structure();
                si2 = null;
                // Test null struct
                return prx.opStruct(si1, si2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(retval.p === null);
                test(retval.e === Test.MyEnum.enum1);
                test(retval.s.s === "");
                test(p3.p === null);
                test(p3.e === Test.MyEnum.enum1);
                test(p3.s.s === "a new string");

                var bsi1 = Ice.Buffer.createNative([ 0x01, 0x11, 0x12, 0x22 ]);
                var bsi2 = Ice.Buffer.createNative([ 0xf1, 0xf2, 0xf3, 0xf4 ]);
                return prx.opByteS(bsi1, bsi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var bsi1 = [ true, true, false ];
                var bsi2 = [ false ];
                return prx.opBoolS(bsi1, bsi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(p3.length == 4);
                test(p3[0]);
                test(p3[1]);
                test(!p3[2]);
                test(!p3[3]);
                test(retval.length == 3);
                test(!retval[0]);
                test(retval[1]);
                test(retval[2]);

                var ssi = [ 1, 2, 3 ];
                var isi = [ 5, 6, 7, 8 ];
                var l1 = new Ice.Long(0, 10);
                var l2 = new Ice.Long(0, 30);
                var l3 = new Ice.Long(0, 20);
                var lsi = [ l1, l2, l3 ];
                return prx.opShortIntLongS(ssi, isi, lsi);
            }
        ).then(r =>
            {
                var [retval, sso, iso, lso] = r;
                var l1 = new Ice.Long(0, 10);
                var l2 = new Ice.Long(0, 30);
                var l3 = new Ice.Long(0, 20);
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

                var fsi = [ 3.14, 1.11 ];
                var dsi = [ 1.1E10, 1.2E10, 1.3E10 ];
                return prx.opFloatDoubleS(fsi, dsi);
            }
        ).then(r =>
            {
                var [retval, fso, dso] = r;
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
                var ssi1 = [ "abc", "de", "fghi" ];
                var ssi2 = [ "xyz" ];
                return prx.opStringS(ssi1, ssi2);
            }
        ).then(r =>
            {
                var [retval, sso] = r;
                test(sso.length === 4);
                test(sso[0] === "abc");
                test(sso[1] === "de");
                test(sso[2] === "fghi");
                test(sso[3] === "xyz");
                test(retval.length === 3);
                test(retval[0] === "fghi");
                test(retval[1] === "de");
                test(retval[2] === "abc");

                var bsi1 =
                    [
                        Ice.Buffer.createNative([ 0x01, 0x11, 0x12 ]),
                        Ice.Buffer.createNative([ 0xff ])
                    ];
                var bsi2 =
                    [
                        Ice.Buffer.createNative([ 0x0e ]),
                        Ice.Buffer.createNative([ 0xf2, 0xf1 ])
                    ];
                return prx.opByteSS(bsi1, bsi2);
            }
        ).then(r =>
            {
                var [retval, bso] = r;
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

                var bsi1 =
                [
                    [ true ],
                    [ false ],
                    [ true, true ]
                ];
                var bsi2 =
                [
                    [ false, false, true ]
                ];
                return prx.opBoolSS(bsi1, bsi2);
            }
        ).then(r =>
            {
                var [retval, bso] = r;
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

                var ssi =
                [
                    [ 1, 2, 5 ],
                    [ 13 ],
                    [ ]
                ];
                var isi =
                [
                    [ 24, 98 ],
                    [ 42 ]
                ];
                var l1 = new Ice.Long(0, 496);
                var l2 = new Ice.Long(0, 1729);
                var lsi =
                [
                    [ l1, l2 ]
                ];
                return prx.opShortIntLongSS(ssi, isi, lsi);

            }
        ).then(r =>
            {
                var [retval, sso, iso, lso] = r;
                var l1 = new Ice.Long(0, 496);
                var l2 = new Ice.Long(0, 1729);

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

                var fsi =
                    [
                        [ 3.14 ],
                        [ 1.11 ],
                        [ ],
                    ];
                var dsi =
                    [
                        [ 1.1E10, 1.2E10, 1.3E10 ]
                    ];
                return prx.opFloatDoubleSS(fsi, dsi);
            }
        ).then(r =>
            {
                var [retval, fso, dso] = r;
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

                var ssi1 =
                    [
                        [ "abc" ],
                        [ "de", "fghi" ]
                    ];
                var ssi2 =
                    [
                        [ ],
                        [ ],
                        [ "xyz" ]
                    ];
                return prx.opStringSS(ssi1, ssi2);
            }
        ).then(r =>
            {
                var [retval, sso] = r;
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

                var sssi1 =
                [
                    [
                        ["abc", "de"],
                        ["xyz"]
                    ],
                    [
                        ["hello"]
                    ]
                ];

                var sssi2 =
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
                return prx.opStringSSS(sssi1, sssi2);
            }
        ).then(r =>
            {
                var [retval, ssso] = r;
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

                var di1 = new Test.ByteBoolD();
                di1.set(10, true);
                di1.set(100, false);
                var di2 = new Test.ByteBoolD();
                di2.set(10, true);
                di2.set(11, false);
                di2.set(101, true);

                return prx.opByteBoolD(di1, di2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var tmp = new Test.ByteBoolD();
                tmp.set(10, true);
                tmp.set(100, false);
                test(Ice.MapUtil.equals(p3, tmp));
                test(retval.size === 4);
                test(retval.get(10) === true);
                test(retval.get(11) === false);
                test(retval.get(100) === false);
                test(retval.get(101) === true);

                var di1 = new Test.ShortIntD();
                di1.set(110, -1);
                di1.set(1100, 123123);
                var di2 = new Test.ShortIntD();
                di2.set(110, -1);
                di2.set(111, -100);
                di2.set(1101, 0);

                return prx.opShortIntD(di1, di2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var tmp = new Test.ShortIntD();
                tmp.set(110, -1);
                tmp.set(1100, 123123);
                test(Ice.MapUtil.equals(p3, tmp));
                test(retval.size === 4);
                test(retval.get(110) === -1);
                test(retval.get(111) === -100);
                test(retval.get(1100) === 123123);
                test(retval.get(1101) === 0);

                var di1 = new Test.LongFloatD();
                di1.set(new Ice.Long(0, 999999110), -1.1);
                di1.set(new Ice.Long(0, 999999111), 123123.2);
                var di2 = new Test.LongFloatD();
                di2.set(new Ice.Long(0, 999999110), -1.1);
                di2.set(new Ice.Long(0, 999999120), -100.4);
                di2.set(new Ice.Long(0, 999999130), 0.5);

                return prx.opLongFloatD(di1, di2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var tmp = new Test.LongFloatD();
                tmp.set(new Ice.Long(0, 999999110), -1.1);
                tmp.set(new Ice.Long(0, 999999111), 123123.2);

                test(p3.equals(tmp, (v1, v2) => (Math.abs(v1) - Math.abs(v2)) <= 0.01));
                test(retval.size === 4);
                test(Math.abs(retval.get(new Ice.Long(0, 999999110))) - Math.abs(-1.1) <= 0.01);
                test(Math.abs(retval.get(new Ice.Long(0, 999999120))) - Math.abs(-100.4) <= 0.01);
                test(retval.get(new Ice.Long(0, 999999111)) - 123123.2 <= 0.01);
                test(retval.get(new Ice.Long(0, 999999130)) - 0.5 <= 0.01);

                var di1 = new Test.StringStringD();
                di1.set("foo", "abc -1.1");
                di1.set("bar", "abc 123123.2");
                var di2 = new Test.StringStringD();
                di2.set("foo", "abc -1.1");
                di2.set("FOO", "abc -100.4");
                di2.set("BAR", "abc 0.5");

                return prx.opStringStringD(di1, di2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var tmp = new Test.StringStringD();
                tmp.set("foo", "abc -1.1");
                tmp.set("bar", "abc 123123.2");
                test(Ice.MapUtil.equals(p3, tmp));
                test(retval.size == 4);
                test(retval.get("foo") === "abc -1.1");
                test(retval.get("FOO") === "abc -100.4");
                test(retval.get("bar") === "abc 123123.2");
                test(retval.get("BAR") === "abc 0.5");

                var di1 = new Test.StringMyEnumD();
                di1.set("abc", Test.MyEnum.enum1);
                di1.set("", Test.MyEnum.enum2);
                var di2 = new Test.StringMyEnumD();
                di2.set("abc", Test.MyEnum.enum1);
                di2.set("qwerty", Test.MyEnum.enum3);
                di2.set("Hello!!", Test.MyEnum.enum2);

                return prx.opStringMyEnumD(di1, di2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var tmp = new Test.StringMyEnumD();
                tmp.set("abc", Test.MyEnum.enum1);
                tmp.set("", Test.MyEnum.enum2);
                test(Ice.MapUtil.equals(p3, tmp));
                test(retval.size === 4);
                test(retval.get("abc") === Test.MyEnum.enum1);
                test(retval.get("qwerty") === Test.MyEnum.enum3);
                test(retval.get("") === Test.MyEnum.enum2);
                test(retval.get("Hello!!") === Test.MyEnum.enum2);

                var di1 = new Test.MyEnumStringD();
                di1.set(Test.MyEnum.enum1, "abc");
                var di2 = new Test.MyEnumStringD();
                di2.set(Test.MyEnum.enum2, "Hello!!");
                di2.set(Test.MyEnum.enum3, "qwerty");

                return prx.opMyEnumStringD(di1, di2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var tmp = new Test.MyEnumStringD();
                tmp.set(Test.MyEnum.enum1, "abc");
                test(Ice.MapUtil.equals(p3, tmp));
                test(retval.size === 3);
                test(retval.get(Test.MyEnum.enum1) === "abc");
                test(retval.get(Test.MyEnum.enum2) === "Hello!!");
                test(retval.get(Test.MyEnum.enum3) === "qwerty");

                var s11 = new Test.MyStruct(1, 1);
                var s12 = new Test.MyStruct(1, 2);
                var di1 = new Test.MyStructMyEnumD();
                di1.set(s11, Test.MyEnum.enum1);
                di1.set(s12, Test.MyEnum.enum2);
                var s22 = new Test.MyStruct(2, 2);
                var s23 = new Test.MyStruct(2, 3);
                var di2 = new Test.MyStructMyEnumD();
                di2.set(s11, Test.MyEnum.enum1);
                di2.set(s22, Test.MyEnum.enum3);
                di2.set(s23, Test.MyEnum.enum2);

                return prx.opMyStructMyEnumD(di1, di2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var tmp = new Test.MyStructMyEnumD();
                var ts11 = new Test.MyStruct(1, 1);
                var ts12 = new Test.MyStruct(1, 2);
                tmp.set(ts11, Test.MyEnum.enum1);
                tmp.set(ts12, Test.MyEnum.enum2);
                test(p3.equals(tmp));
                var ts22 = new Test.MyStruct(2, 2);
                var ts23 = new Test.MyStruct(2, 3);
                test(retval.size === 4);
                test(retval.get(ts11) === Test.MyEnum.enum1);
                test(retval.get(ts12) === Test.MyEnum.enum2);
                test(retval.get(ts22) === Test.MyEnum.enum3);
                test(retval.get(ts23) === Test.MyEnum.enum2);

                var ds1 = new Test.ByteBoolD();
                ds1.set(10, true);
                ds1.set(100, false);
                var ds2 = new Test.ByteBoolD();
                ds2.set(10, true);
                ds2.set(11, false);
                ds2.set(101, true);
                var ds3 = new Test.ByteBoolD();
                ds3.set(100, false);
                ds3.set(101, false);

                return prx.opByteBoolDS([ds1, ds2], [ds3]);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var di1 = new Test.ShortIntD();
                di1.set(110, -1);
                di1.set(1100, 123123);
                var di2 = new Test.ShortIntD();
                di2.set(110, -1);
                di2.set(111, -100);
                di2.set(1101, 0);
                var di3 = new Test.ShortIntD();
                di3.set(100, -1001);

                return prx.opShortIntDS([di1, di2], [di3]);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var di1 = new Test.LongFloatD();
                di1.set(new Ice.Long(0, 999999110), -1.1);
                di1.set(new Ice.Long(0, 999999111), 123123.2);
                var di2 = new Test.LongFloatD();
                di2.set(new Ice.Long(0, 999999110), -1.1);
                di2.set(new Ice.Long(0, 999999120), -100.4);
                di2.set(new Ice.Long(0, 999999130), 0.5);
                var di3 = new Test.LongFloatD();
                di3.set(new Ice.Long(0, 999999140), 3.14);

                return prx.opLongFloatDS([di1, di2], [di3]);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var di1 = new Test.StringStringD();
                di1.set("foo", "abc -1.1");
                di1.set("bar", "abc 123123.2");
                var di2 = new Test.StringStringD();
                di2.set("foo", "abc -1.1");
                di2.set("FOO", "abc -100.4");
                di2.set("BAR", "abc 0.5");
                var di3 = new Test.StringStringD();
                di3.set("f00", "ABC -3.14");

                return prx.opStringStringDS([di1, di2], [di3]);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var di1 = new Test.StringMyEnumD();
                di1.set("abc", Test.MyEnum.enum1);
                di1.set("", Test.MyEnum.enum2);
                var di2 = new Test.StringMyEnumD();
                di2.set("abc", Test.MyEnum.enum1);
                di2.set("qwerty", Test.MyEnum.enum3);
                di2.set("Hello!!", Test.MyEnum.enum2);
                var di3 = new Test.StringMyEnumD();
                di3.set("Goodbye", Test.MyEnum.enum1);

                return prx.opStringMyEnumDS([di1, di2], [di3]);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var di1 = new Test.MyEnumStringD();
                di1.set(Test.MyEnum.enum1, "abc");
                var di2 = new Test.MyEnumStringD();
                di2.set(Test.MyEnum.enum2, "Hello!!");
                di2.set(Test.MyEnum.enum3, "qwerty");
                var di3 = new Test.MyEnumStringD();
                di3.set(Test.MyEnum.enum1, "Goodbye");

                return prx.opMyEnumStringDS([di1, di2], [di3]);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var s11 = new Test.MyStruct(1, 1);
                var s12 = new Test.MyStruct(1, 2);
                var di1 = new Test.MyStructMyEnumD();
                di1.set(s11, Test.MyEnum.enum1);
                di1.set(s12, Test.MyEnum.enum2);

                var s22 = new Test.MyStruct(2, 2);
                var s23 = new Test.MyStruct(2, 3);
                var di2 = new Test.MyStructMyEnumD();
                di2.set(s11, Test.MyEnum.enum1);
                di2.set(s22, Test.MyEnum.enum3);
                di2.set(s23, Test.MyEnum.enum2);

                var di3 = new Test.MyStructMyEnumD();
                di3.set(s23, Test.MyEnum.enum3);

                return prx.opMyStructMyEnumDS([di1, di2], [di3]);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                var s11 = new Test.MyStruct(1, 1);
                var s12 = new Test.MyStruct(1, 2);
                var s22 = new Test.MyStruct(2, 2);
                var s23 = new Test.MyStruct(2, 3);

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

                var sdi1 = new Test.ByteByteSD();
                sdi1.set(0x01, Ice.Buffer.createNative([0x01, 0x11]));
                sdi1.set(0x22, Ice.Buffer.createNative([0x12]));
                var sdi2 = new Test.ByteByteSD();
                sdi2.set(0xf1, Ice.Buffer.createNative([0xf2, 0xf3]));

                return prx.opByteByteSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var si1 = [true, false];
                var si2 = [false, true, true];

                var sdi1 = new Test.BoolBoolSD();
                sdi1.set(false, si1);
                sdi1.set(true, si2);
                var sdi2 = new Test.BoolBoolSD();
                sdi2.set(false, si1);

                return prx.opBoolBoolSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var sdi1 = new Test.ShortShortSD();
                var sdi2 = new Test.ShortShortSD();

                var si1 = [1, 2, 3];
                var si2 = [4, 5];
                var si3 = [6, 7];

                sdi1.set(1, si1);
                sdi1.set(2, si2);
                sdi2.set(4, si3);

                return prx.opShortShortSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var sdi1 = new Test.IntIntSD();
                var sdi2 = new Test.IntIntSD();

                var si1 = [100, 200, 300];
                var si2 = [400, 500];
                var si3 = [600, 700];

                sdi1.set(100, si1);
                sdi1.set(200, si2);
                sdi2.set(400, si3);

                return prx.opIntIntSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var sdi1 = new Test.LongLongSD();
                var sdi2 = new Test.LongLongSD();

                var si1 = [new Ice.Long(0, 999999110), new Ice.Long(0, 999999111), new Ice.Long(0, 999999110)];
                var si2 = [new Ice.Long(0, 999999120), new Ice.Long(0, 999999130)];
                var si3 = [new Ice.Long(0, 999999110), new Ice.Long(0, 999999120)];

                sdi1.set(new Ice.Long(0, 999999990), si1);
                sdi1.set(new Ice.Long(0, 999999991), si2);
                sdi2.set(new Ice.Long(0, 999999992), si3);

                return prx.opLongLongSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var sdi1 = new Test.StringFloatSD();
                var sdi2 = new Test.StringFloatSD();

                var si1 = [-1.1, 123123.2, 100.0];
                var si2 = [42.24, -1.61];
                var si3 = [-3.14, 3.14];

                sdi1.set("abc", si1);
                sdi1.set("ABC", si2);
                sdi2.set("aBc", si3);

                return prx.opStringFloatSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
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

                var sdi1 = new Test.StringDoubleSD();
                var sdi2 = new Test.StringDoubleSD();

                var si1 = [1.1E10, 1.2E10, 1.3E10];
                var si2 = [1.4E10, 1.5E10];
                var si3 = [1.6E10, 1.7E10];

                sdi1.set("Hello!!", si1);
                sdi1.set("Goodbye",  si2);
                sdi2.set("", si3);

                return prx.opStringDoubleSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(p3.size === 1);
                test(p3.get("").length === 2);
                test(p3.get("")[0] === 1.6E10);
                test(p3.get("")[1] === 1.7E10);
                test(retval.size=== 3);
                test(retval.get("Hello!!").length === 3);
                test(retval.get("Hello!!")[0] === 1.1E10);
                test(retval.get("Hello!!")[1] === 1.2E10);
                test(retval.get("Hello!!")[2] === 1.3E10);
                test(retval.get("Goodbye").length === 2);
                test(retval.get("Goodbye")[0] === 1.4E10);
                test(retval.get("Goodbye")[1] === 1.5E10);
                test(retval.get("").length=== 2);
                test(retval.get("")[0] === 1.6E10);
                test(retval.get("")[1] === 1.7E10);

                var sdi1 = new Test.StringStringSD();
                var sdi2 = new Test.StringStringSD();

                var si1 = ["abc", "de", "fghi"];
                var si2 = ["xyz", "or"];
                var si3 = ["and", "xor"];

                sdi1.set("abc", si1);
                sdi1.set("def", si2);
                sdi2.set("ghi", si3);

                return prx.opStringStringSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(p3.size === 1);
                test(p3.get("ghi").length ===2);
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

                var sdi1 = new Test.MyEnumMyEnumSD();
                var sdi2 = new Test.MyEnumMyEnumSD();

                var si1 = [Test.MyEnum.enum1, Test.MyEnum.enum1, Test.MyEnum.enum2];
                var si2 = [Test.MyEnum.enum1, Test.MyEnum.enum2];
                var si3 = [Test.MyEnum.enum3, Test.MyEnum.enum3];

                sdi1.set(Test.MyEnum.enum3, si1);
                sdi1.set(Test.MyEnum.enum2, si2);
                sdi2.set(Test.MyEnum.enum1, si3);

                return prx.opMyEnumMyEnumSD(sdi1, sdi2);
            }
        ).then(r =>
            {
                var [retval, p3] = r;
                test(p3.size == 1);
                test(p3.get(Test.MyEnum.enum1).length == 2);
                test(p3.get(Test.MyEnum.enum1)[0] == Test.MyEnum.enum3);
                test(p3.get(Test.MyEnum.enum1)[1] == Test.MyEnum.enum3);
                test(retval.size== 3);
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

                var next = function(n)
                {
                    var lengths = [ 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 ];
                    if(n < lengths.length)
                    {
                        var len = lengths[n];
                        var s = [];
                        for(var i = 0; i < len; ++i)
                        {
                            s.push(i);
                        }

                        return prx.opIntS(s).then(
                            function(retval)
                            {
                                test(retval.length === len);
                                for(var i = 0; i < len; ++i)
                                {
                                    test(retval[i] === -i);
                                }
                                return next(n + 1);
                            });
                    }
                };

                return next(0);
            }
        ).then(() =>
            {
                test(prx.ice_getContext().size === 0);
                return prx.opContext();
            }
        ).then(c =>
            {
                test(!Ice.MapUtil.equals(c, ctx));
                return prx.opContext(ctx);
            }
        ).then(c =>
            {
                test(Ice.MapUtil.equals(c, ctx));
                return Test.MyClassPrx.checkedCast(prx.ice_context(ctx));
            }
        ).then(p2 =>
            {
                test(Ice.MapUtil.equals(p2.ice_getContext(), ctx));
                return p2.opContext().then(c =>
                    {
                        test(Ice.MapUtil.equals(c, ctx));
                        return p2.opContext(ctx);
                    }
                ).then(c =>
                    {
                        test(Ice.MapUtil.equals(c, ctx));
                        if(!bidir)
                        {
                            var p3, ic;
                            return Ice.Promise.try(() =>
                                {
                                    test(Ice.MapUtil.equals(c, ctx));

                                    //
                                    // Test implicit context propagation
                                    //

                                    var initData = new Ice.InitializationData();
                                    initData.properties = communicator.getProperties().clone();
                                    initData.properties.setProperty("Ice.ImplicitContext", "Shared");
                                    ic = Ice.initialize(initData);
                                    p3 = Test.MyClassPrx.uncheckedCast(ic.stringToProxy("test:default -p 12010"));
                                    ic.getImplicitContext().setContext(ctx);
                                    test(Ice.MapUtil.equals(ic.getImplicitContext().getContext(), ctx));
                                    return p3.opContext();
                                }
                            ).then(c =>
                                {
                                    ic.getImplicitContext().put("zero", "ZERO");
                                    return p3.opContext();
                                }
                            ).then(c =>
                                {
                                    test(Ice.MapUtil.equals(c, ic.getImplicitContext().getContext()));

                                    ctx = ic.getImplicitContext().getContext();

                                    var prxContext = new Ice.Context();
                                    prxContext.set("one", "UN");
                                    prxContext.set("four", "QUATRE");

                                    combined = new Ice.Context(ctx);
                                    for(let [key, value] of prxContext)
                                    {
                                        combined.set(key, value);
                                    }
                                    test(combined.get("one") === "UN");

                                    p3 = Test.MyClassPrx.uncheckedCast(p3.ice_context(prxContext));

                                    ic.getImplicitContext().setContext(null);

                                    return p3.opContext();
                                }
                            ).then(c =>
                                {
                                    test(Ice.MapUtil.equals(c, p3.ice_getContext()));
                                    ic.getImplicitContext().setContext(ctx);

                                    return p3.opContext();
                                }
                            ).then(c =>
                                {
                                    test(Ice.MapUtil.equals(c, combined));

                                    return ic.destroy();
                                });
                        }
                    });
            }
        ).then(() =>
            {
                var d = 1278312346.0 / 13.0;
                var ds = [];
                for(var i = 0; i < 5; i++)
                {
                    ds[i] = d;
                }

                return prx.opDoubleMarshaling(d, ds);
            }
        ).then(() => prx.opIdempotent()
        ).then(() => prx.opNonmutating()
        ).then(() => Test.MyDerivedClassPrx.checkedCast(prx)
        ).then(d => d.opDerived()
        ).then(() =>
            {
                var d = Test.MyDerivedClassPrx.uncheckedCast(prx);
                var s = new Test.MyStruct1();
                s.tesT = "Test.MyStruct1.s";
                s.myClass = null;
                s.myStruct1 = "Test.MyStruct1.myStruct1";

                var c = new Test.MyClass1();
                c.tesT = "Test.MyClass1.testT";
                c.myClass = null;
                c.myClass1 = "Test.MyClass1.myClass1";

                return Ice.Promise.all(
                    [
                        prx.opByte1(0xFF),
                        prx.opShort1(0x7FFF),
                        prx.opInt1(0x7FFFFFFF),
                        prx.opLong1(new Ice.Long(0x7FFFFFFF, 0xFFFFFFFF)),
                        prx.opFloat1(1.0),
                        prx.opDouble1(1.0),
                        prx.opString1("opString1"),
                        prx.opStringS1(null),
                        prx.opByteBoolD1(null),
                        prx.opStringS2(null),
                        prx.opByteBoolD2(null),
                        d.opMyStruct1(s),
                        d.opMyClass1(c)
                    ]
                ).then(r =>
                    {
                        test(r[0] == 0xFF);
                        test(r[1] == 0x7FFF);
                        test(r[2] == 0x7FFFFFFF);
                        var l = r[3];
                        test(l.high == 0x7FFFFFFF && l.low == 0xFFFFFFFF);
                        test(r[4] == 1.0);
                        test(r[5] == 1.0);
                        test(r[6] == "opString1");
                        test(r[7].length === 0);
                        test(r[8].size === 0);
                        test(r[9].length === 0);
                        test(r[10].size === 0);

                        s = r[11];
                        test(s.tesT == "Test.MyStruct1.s");
                        test(s.myClass === null);
                        test(s.myStruct1 == "Test.MyStruct1.myStruct1");

                        c = r[12];
                        test(c.tesT == "Test.MyClass1.testT");
                        test(c.myClass === null);
                        test(c.myClass1 == "Test.MyClass1.myClass1");
                    });
            }
        ).then(() => prx.opMStruct1()
        ).then((result) =>
            {
                result.e = Test.MyEnum.enum3;
                return prx.opMStruct2(result);
            }
        ).then((result) =>
            {
                let [r, p2] = result;
                test(p2.e == Test.MyEnum.enum3 && r.e == Test.MyEnum.enum3);
                return prx.opMSeq1();
            }
        ).then((result) =>
            {
                return prx.opMSeq2(["test"]);
            }
        ).then((result) =>
            {
                let [r, p2] = result;
                test(p2[0] == "test" && r[0] == "test");
                return prx.opMDict1();
            }
        ).then((result) =>
            {
                return prx.opMDict2(new Map([["test", "test"]]));
            }
        ).then((result) =>
            {
                let [r, p2] = result;
                test(r.get("test") == "test" && p2.get("test") == "test");
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    exports.Twoways = { run: run };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
