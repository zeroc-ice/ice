// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("icejs").Ice;
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
                    p.fail(err);
                    throw err;
                }
            }
        };

        var si1, si2, ctx, combined;
        Ice.Promise.try(
            function()
            {
                ctx = new Ice.Context();
                ctx.set("one", "ONE");
                ctx.set("two", "TWO");
                ctx.set("three", "THREE");

                return prx.ice_ping();
            }
        ).then(
            function()
            {
                return prx.ice_isA(Test.MyClass.ice_staticId());
            }
        ).then(
            function(b)
            {
                test(b);
                return prx.ice_id();
            }
        ).then(
            function(id)
            {
                test(id === Test.MyDerivedClass.ice_staticId());
                return prx.ice_ids();
            }
        ).then(
            function(ids)
            {
                test(ids.length === 3);
                return prx.opVoid();
            }
        ).then(
            function()
            {
                return prx.opByte(0xff, 0x0f);
            }
        ).then(
            function(retval, p3)
            {
                test(p3 === 0xf0);
                test(retval === 0xff);
                return prx.opBool(true, false);
            }
        ).then(
            function(retval, p3)
            {
                test(p3);
                test(!retval);
                var lo = new Ice.Long(0, 12);
                return prx.opShortIntLong(10, 11, lo);
            }
        ).then(
            function(retval, s, i, l)
            {
                var lo = new Ice.Long(0, 12);
                test(s === 10);
                test(i === 11);
                test(l.equals(lo));
                test(retval.equals(lo));
                return prx.opFloatDouble(3.14, 1.1E10);
            }
        ).then(
            function(retval, f, d)
            {
                test((f - 3.14) <= 0.01);
                test(d == 1.1E10);
                test(retval == 1.1E10);
                return prx.opString("hello", "world");
            }
        ).then(
            function(retval, p3)
            {
                test(p3 === "world hello");
                test(retval === "hello world");
                return prx.opMyEnum(Test.MyEnum.enum2);
            }
        ).then(
            function(retval, p2)
            {
                test(p2 === Test.MyEnum.enum2);
                test(retval === Test.MyEnum.enum3);
                return prx.opMyClass(prx);
            }
        ).then(
            function(retval, p2, p3)
            {
                test(p2.ice_getIdentity().equals(communicator.stringToIdentity("test")));
                test(p3.ice_getIdentity().equals(communicator.stringToIdentity("noSuchIdentity")));
                test(retval.ice_getIdentity().equals(communicator.stringToIdentity("test")));

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
        ).then(
            function(retval, p3)
            {
                test(retval.p === null);
                test(retval.e === Test.MyEnum.enum2);
                test(retval.s.s === "def");
                test(p3.p !== null);
                test(p3.e === Test.MyEnum.enum3);
                test(p3.s.s === "a new string");

                var bsi1 = Ice.Buffer.createNative([ 0x01, 0x11, 0x12, 0x22 ]);
                var bsi2 = Ice.Buffer.createNative([ 0xf1, 0xf2, 0xf3, 0xf4 ]);
                return prx.opByteS(bsi1, bsi2);
            }
        ).then(
            function(retval, p3)
            {
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
        ).then(
            function(retval, p3)
            {
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
        ).then(
            function(retval, sso, iso, lso)
            {
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
        ).then(
            function(retval, fso, dso)
            {
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
        ).then(
            function(retval, sso)
            {
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
        ).then(
            function(retval, bso)
            {
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
        ).then(
            function(retval, bso)
            {
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
                ]
                return prx.opShortIntLongSS(ssi, isi, lsi)

            }
        ).then(
            function(retval, sso, iso, lso)
            {
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
        ).then(
            function(retval, fso, dso)
            {
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
        ).then(
            function(retval, sso)
            {
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
        ).then(
            function(retval, ssso)
            {
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
        ).then(
            function(retval, p3)
            {
                var tmp = new Test.ByteBoolD();
                tmp.set(10, true);
                tmp.set(100, false);
                test(p3.equals(tmp));
                test(retval.size === 4);
                test(retval.get(10) == true);
                test(retval.get(11) == false);
                test(retval.get(100) == false);
                test(retval.get(101) == true);

                var di1 = new Test.ShortIntD();
                di1.set(110, -1);
                di1.set(1100, 123123);
                var di2 = new Test.ShortIntD();
                di2.set(110, -1);
                di2.set(111, -100);
                di2.set(1101, 0);

                return prx.opShortIntD(di1, di2);
            }
        ).then(
            function(retval, p3)
            {
                var tmp = new Test.ShortIntD();
                tmp.set(110, -1);
                tmp.set(1100, 123123);
                test(p3.equals(tmp));
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
        ).then(
            function(retval, p3)
            {
                var tmp = new Test.LongFloatD();
                tmp.set(new Ice.Long(0, 999999110), -1.1);
                tmp.set(new Ice.Long(0, 999999111), 123123.2);

                test(p3.equals(tmp, 
                               function(v1, v2) { 
                                return (Math.abs(v1) - Math.abs(v2)) <= 0.01; }));
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
        ).then(
            function(retval, p3)
            {
                var tmp = new Test.StringStringD();
                tmp.set("foo", "abc -1.1");
                tmp.set("bar", "abc 123123.2");
                test(p3.equals(tmp));
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
        ).then(
            function(retval, p3)
            {
                var tmp = new Test.StringMyEnumD();
                tmp.set("abc", Test.MyEnum.enum1);
                tmp.set("", Test.MyEnum.enum2);
                test(p3.equals(tmp));
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
        ).then(
            function(retval, p3)
            {
                var tmp = new Test.MyEnumStringD();
                tmp.set(Test.MyEnum.enum1, "abc");
                test(p3.equals(tmp));
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
        ).then(
            function(retval, p3)
            {
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

                var promise2 = new Ice.Promise();
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
        ).then(
            function()
            {
                test(prx.ice_getContext().size === 0);

                return prx.opContext();
            }
        ).then(
            function(c)
            {
                test(!c.equals(ctx));
                return prx.opContext(ctx);
            }
        ).then(
            function(c)
            {
                test(c.equals(ctx));
                return Test.MyClassPrx.checkedCast(prx.ice_context(ctx));
            }
        ).then(
            function(p2)
            {
                test(p2.ice_getContext().equals(ctx));
                return p2.opContext();
            }
        ).then(
            function(c, r)
            {
                test(c.equals(ctx));
                return r.proxy.opContext(ctx);
            }
        ).then(
            function(c, r)
            {
                test(c.equals(ctx));
                if(!bidir)
                {
                    return Ice.Promise.try(
                        function()
                        {
                            test(c.equals(ctx));

                            //
                            // Test implicit context propagation
                            //

                            var initData = new Ice.InitializationData();
                            initData.properties = communicator.getProperties().clone();
                            initData.properties.setProperty("Ice.ImplicitContext", "Shared");
                            var ic = Ice.initialize(initData);
                            var p3 = Test.MyClassPrx.uncheckedCast(ic.stringToProxy("test:default -p 12010"));
                            ic.getImplicitContext().setContext(ctx);
                            test(ic.getImplicitContext().getContext().equals(ctx));
                            return p3.opContext();
                        }
                    ).then(
                        function(c, r)
                        {
                            r.communicator.getImplicitContext().put("zero", "ZERO");
                            return r.proxy.opContext();
                        }
                    ).then(
                        function(c, r)
                        {
                            test(c.equals(r.communicator.getImplicitContext().getContext()));

                            ctx = r.communicator.getImplicitContext().getContext();

                            var prxContext = new Ice.Context();
                            prxContext.set("one", "UN");
                            prxContext.set("four", "QUATRE");

                            combined = new Ice.Context(ctx);
                            combined.merge(prxContext);
                            test(combined.get("one") === "UN");

                            var p3 = Test.MyClassPrx.uncheckedCast(r.proxy.ice_context(prxContext));

                            r.communicator.getImplicitContext().setContext(null);

                            return p3.opContext();
                        }
                    ).then(
                        function(c, r)
                        {
                            test(c.equals(r.proxy.ice_getContext()));
                            r.communicator.getImplicitContext().setContext(ctx);

                            return r.proxy.opContext();
                        }
                    ).then(
                        function(c, r)
                        {
                            test(c.equals(combined));

                            return r.communicator.destroy();
                        });
                }
            }
        ).then(
            function()
            {
                var d = 1278312346.0 / 13.0;
                var ds = [];
                for(var i = 0; i < 5; i++)
                {
                    ds[i] = d;
                }

                return prx.opDoubleMarshaling(d, ds);
            }
        ).then(
            function()
            {
                return prx.opIdempotent();
            }
        ).then(
            function()
            {
                return prx.opNonmutating();
            }
        ).then(
            function()
            {
                return Test.MyDerivedClassPrx.checkedCast(prx);
            }
        ).then(
            function(d)
            {
                return d.opDerived();
            }
        ).then(
            function()
            {
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            }
        );
        return p;
    };

    exports.Twoways = { run: run };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : window.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : window));

