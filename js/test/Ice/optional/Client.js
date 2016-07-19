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
    var ClientPrivate = require("../optional/ClientPrivate").Test;

    var Promise = Ice.Promise;
    var ArrayUtil = Ice.ArrayUtil;

    var allTests = function(out, communicator, Test)
    {
        var failCB = function(){ test(false); };

        var ref, base, mo1, mo6, mo8, oo1, initial, initial2;

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

        Promise.try(
            function()
            {
                out.write("testing stringToProxy... ");
                ref = "initial:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");

                oo1 = new Test.OneOptional();
                oo1.a = 15;

                var initial, initial2;

                out.write("testing checked cast... ");
                return Test.InitialPrx.checkedCast(base);
            }
        ).then(
            function(obj)
            {
                initial = obj;
                test(initial !== null);
                test(initial.equals(base));
                out.writeLine("ok");
                out.write("testing marshaling... ");

                return initial.pingPong(new Test.OneOptional());
            }
        ).then(
            function(oo4)
            {
                test(oo4.a === undefined);
                return initial.pingPong(oo1);
            }
        ).then(
            function(oo5)
            {
                test(oo5.a === oo1.a);
                return initial.pingPong(new Test.MultiOptional());
            }
        ).then(
            function(mo4)
            {
                test(mo4.a === undefined);
                test(mo4.b === undefined);
                test(mo4.c === undefined);
                test(mo4.d === undefined);
                test(mo4.e === undefined);
                test(mo4.f === undefined);
                test(mo4.g === undefined);
                test(mo4.h === undefined);
                test(mo4.i === undefined);
                test(mo4.j === undefined);
                test(mo4.k === undefined);
                test(mo4.bs === undefined);
                test(mo4.ss === undefined);
                test(mo4.iid === undefined);
                test(mo4.sid === undefined);
                test(mo4.fs === undefined);
                test(mo4.vs === undefined);

                test(mo4.shs === undefined);
                test(mo4.es === undefined);
                test(mo4.fss === undefined);
                test(mo4.vss === undefined);
                test(mo4.oos === undefined);
                test(mo4.oops === undefined);

                test(mo4.ied === undefined);
                test(mo4.ifsd === undefined);
                test(mo4.ivsd === undefined);
                test(mo4.iood === undefined);
                test(mo4.ioopd === undefined);

                test(mo4.bos === undefined);

                test(mo4.ser === undefined);


                mo1 = new Test.MultiOptional();
                mo1.a = 15;
                mo1.b = true;
                mo1.c = 19;
                mo1.d = 78;
                mo1.e = new Ice.Long(0, 99);
                mo1.f = 5.5;
                mo1.g = 1.0;
                mo1.h = "test";
                mo1.i = Test.MyEnum.MyEnumMember;
                mo1.j = Test.MultiOptionalPrx.uncheckedCast(communicator.stringToProxy("test"));
                mo1.k = mo1;
                mo1.bs = Ice.Buffer.createNative([5]);
                mo1.ss = ["test", "test2"];
                mo1.iid = new Ice.HashMap();
                mo1.iid.set(4, 3);
                mo1.sid = new Ice.HashMap();
                mo1.sid.set("test", 10);
                mo1.fs = new Test.FixedStruct();
                mo1.fs.m = 78;
                mo1.vs = new Test.VarStruct();
                mo1.vs.m = "hello";

                mo1.shs = [1];
                mo1.es = [Test.MyEnum.MyEnumMember, Test.MyEnum.MyEnumMember];
                mo1.fss = [mo1.fs];
                mo1.vss = [mo1.vs];
                mo1.oos = [oo1];
                mo1.oops = [Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"))];

                mo1.ied = new Ice.HashMap();
                mo1.ied.set(4, Test.MyEnum.MyEnumMember);
                mo1.ifsd = new Ice.HashMap();
                mo1.ifsd.set(4, mo1.fs);
                mo1.ivsd = new Ice.HashMap();
                mo1.ivsd.set(5, mo1.vs);
                mo1.iood = new Ice.HashMap();
                mo1.iood.set(5, new Test.OneOptional(15));
                mo1.ioopd = new Ice.HashMap();
                mo1.ioopd.set(5, Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")));

                mo1.bos = [false, true, false];

                return initial.pingPong(mo1);
            }
        ).then(
            function(mo5)
            {
                test(mo1.a == mo5.a);
                test(mo1.b == mo5.b);
                test(mo1.c == mo5.c);
                test(mo1.d == mo5.d);
                test(mo1.e.equals(mo5.e));
                test(mo1.f == mo5.f);
                test(mo1.g == mo5.g);
                test(mo1.h == mo5.h);
                test(mo1.i == mo5.i);
                test(mo1.j.equals(mo5.j));
                test(mo5.k == mo5);
                test(ArrayUtil.equals(mo5.bs, mo1.bs));
                test(ArrayUtil.equals(mo5.ss, mo1.ss));
                test(mo5.iid.get(4) == 3);
                test(mo5.sid.get("test") == 10);
                test(mo5.fs.equals(mo1.fs));
                test(mo5.vs.equals(mo1.vs));
                test(ArrayUtil.equals(mo5.shs, mo1.shs));
                test(mo5.es[0] == Test.MyEnum.MyEnumMember && mo5.es[1] == Test.MyEnum.MyEnumMember);
                test(mo5.fss[0].equals(new Test.FixedStruct(78)));
                test(mo5.vss[0].equals(new Test.VarStruct("hello")));
                test(mo5.oos[0].a == 15);
                test(mo5.oops[0].equals(Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"))));

                test(mo5.ied.get(4) == Test.MyEnum.MyEnumMember);
                test(mo5.ifsd.get(4).equals(new Test.FixedStruct(78)));
                test(mo5.ivsd.get(5).equals(new Test.VarStruct("hello")));
                test(mo5.iood.get(5).a == 15);
                test(mo5.ioopd.get(5).equals(Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"))));

                test(ArrayUtil.equals(mo5.bos, [false, true, false]));

                // Clear the first half of the optional parameters
                mo6 = new Test.MultiOptional();
                mo6.b = mo5.b;
                mo6.d = mo5.d;
                mo6.f = mo5.f;
                mo6.h = mo5.h;
                mo6.j = mo5.j;
                mo6.bs = mo5.bs;
                mo6.iid = mo5.iid;
                mo6.fs = mo5.fs;
                mo6.shs = mo5.shs;
                mo6.fss = mo5.fss;
                mo6.oos = mo5.oos;
                mo6.ifsd = mo5.ifsd;
                mo6.iood = mo5.iood;
                mo6.bos = mo5.bos;

                return initial.pingPong(mo6);
            }
        ).then(
            function(mo7)
            {
                test(mo7.a === undefined);
                test(mo7.b == mo1.b);
                test(mo7.c === undefined );
                test(mo7.d == mo1.d);
                test(mo7.e === undefined);
                test(mo7.f == mo1.f);
                test(mo7.g === undefined);
                test(mo7.h == mo1.h);
                test(mo7.i === undefined);
                test(mo7.j.equals(mo1.j));
                test(mo7.k === undefined);
                test(ArrayUtil.equals(mo7.bs, mo1.bs));
                test(mo7.ss === undefined);
                test(mo7.iid.get(4) == 3);
                test(mo7.sid === undefined);
                test(mo7.fs.equals(mo1.fs));
                test(mo7.vs === undefined);
                test(ArrayUtil.equals(mo7.shs, mo1.shs));
                test(mo7.es === undefined);
                test(mo7.fss[0].equals(new Test.FixedStruct(78)));
                test(mo7.vss === undefined);
                test(mo7.oos[0].a == 15);
                test(mo7.oops === undefined);

                test(mo7.ied === undefined);
                test(mo7.ifsd.get(4).equals(new Test.FixedStruct(78)));
                test(mo7.ivsd === undefined);
                test(mo7.iood.get(5).a == 15);
                test(mo7.ioopd === undefined);

                test(ArrayUtil.equals(mo7.bos, [false, true, false]));

                // Clear the second half of the optional parameters
                mo8 = new Test.MultiOptional();
                mo8.a = mo1.a;
                mo8.c = mo1.c;
                mo8.e = mo1.e;
                mo8.g = mo1.g;
                mo8.i = mo1.i;
                mo8.k = mo8;
                mo8.ss = mo1.ss;
                mo8.sid = mo1.sid;
                mo8.vs = mo1.vs;

                mo8.es = mo1.es;
                mo8.vss = mo1.vss;
                mo8.oops = mo1.oops;

                mo8.ied = mo1.ied;
                mo8.ivsd = mo1.ivsd;
                mo8.ioopd = mo1.ioopd;

                return initial.pingPong(mo8);
            }
        ).then(
            function(mo9)
            {
                test(mo9.a == mo1.a);
                test(mo9.b === undefined);
                test(mo9.c == mo1.c);
                test(mo9.d === undefined);
                test(mo9.e.equals(mo1.e));
                test(mo9.f === undefined);
                test(mo9.g == mo1.g);
                test(mo9.h === undefined);
                test(mo9.i == mo1.i);
                test(mo9.j === undefined);
                test(mo9.k == mo9);
                test(mo9.bs === undefined);
                test(ArrayUtil.equals(mo9.ss, mo1.ss));
                test(mo9.iid === undefined);
                test(mo9.sid.get("test") == 10);
                test(mo9.fs === undefined);
                test(mo9.vs.equals(mo1.vs));

                test(mo9.shs === undefined);
                test(mo9.es[0] == Test.MyEnum.MyEnumMember && mo9.es[1] == Test.MyEnum.MyEnumMember);
                test(mo9.fss === undefined);
                test(mo9.vss[0].equals(new Test.VarStruct("hello")));
                test(mo9.oos === undefined);
                test(mo9.oops[0].equals(Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"))));

                test(mo9.ied.get(4) == Test.MyEnum.MyEnumMember);
                test(mo9.ifsd === undefined);
                test(mo9.ivsd.get(5).equals(new Test.VarStruct("hello")));
                test(mo9.iood === undefined);
                test(mo9.ioopd.get(5).equals(Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"))));

                test(mo9.bos === undefined);

                //
                // Use the 1.0 encoding with operations whose only class parameters are optional.
                //
                initial2 = initial.ice_encodingVersion(Ice.Encoding_1_0);
                var oo = new Test.OneOptional(53);

                return initial.sendOptionalClass(true, oo).then(
                        function()
                        {
                            return initial2.sendOptionalClass(true, oo);
                        }
                    ).then(
                        function()
                        {
                            return initial.returnOptionalClass(true);
                        }
                    ).then(
                        function(oo1)
                        {
                            test(oo1 !== undefined && oo1.a == 53);
                            return initial2.returnOptionalClass(true);
                        }
                    ).then(
                        function(oo1)
                        {
                            test(oo1 === undefined);
                        }
                    );
            }
        ).then(
            function()
            {
                var g = new Test.G();
                g.gg1Opt = new Test.G1("gg1Opt");
                g.gg2 = new Test.G2(new Ice.Long(0, 10));
                g.gg2Opt = new Test.G2(new Ice.Long(0, 20));
                g.gg1 = new Test.G1("gg1");
                return initial.opG(g);
            }
        ).then(
            function(g)
            {
                test(g.gg1Opt.a == "gg1Opt");
                test(g.gg2.a.equals(new Ice.Long(0, 10)));
                test(g.gg2Opt.a.equals(new Ice.Long(0, 20)));
                test(g.gg1.a == "gg1");
            }
        ).then(
            function()
            {
                var init2 = ClientPrivate.Initial2Prx.uncheckedCast(initial);
                return init2.opVoid(5, "test");
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing marshaling of large containers with fixed size elements... ");
                var mc = new Test.MultiOptional();

                mc.bs = Ice.Buffer.createNative(new Array(1000));
                mc.shs = new Array(300);

                var i;
                mc.fss = [];
                for(i = 0; i < 300; ++i)
                {
                    mc.fss[i] = new Test.FixedStruct();
                }

                mc.ifsd = new Ice.HashMap();
                for(i = 0; i < 300; ++i)
                {
                    mc.ifsd.set(i, new Test.FixedStruct());
                }
                return initial.pingPong(mc);
            }
        ).then(
            function(mc)
            {
                test(mc.bs.length == 1000);
                test(mc.shs.length == 300);
                test(mc.fss.length == 300);
                test(mc.ifsd.size == 300);

                out.writeLine("ok");
                out.write("testing tag marshaling... ");
                return initial.pingPong(new Test.B());
            }
        ).then(
            function(b)
            {
                test(b.ma === undefined);
                test(b.mb === undefined);
                test(b.mc === undefined);

                b.ma = 10;
                b.mb = 11;
                b.mc = 12;
                b.md = 13;

                return initial.pingPong(b);
            }
        ).then(
            function(b)
            {
                test(b.ma == 10);
                test(b.mb == 11);
                test(b.mc == 12);
                test(b.md == 13);

                out.writeLine("ok");
                out.write("testing marshaling of objects with optional objects... ");

                var f = new Test.F();
                f.af = new Test.A();
                f.ae = f.af;
                return initial.pingPong(f);
            }
        ).then(
            function(f)
            {
                test(f.ae === f.af);

                out.writeLine("ok");
                out.write("testing optional with default values... ");

                return initial.pingPong(new Test.WD());
            }
        ).then(
            function(wd)
            {
                test(wd.a == 5);
                test(wd.s == "test");
                wd.a = undefined;
                wd.s = undefined;
                return initial.pingPong(wd);
            }
        ).then(
            function(wd)
            {
                test(wd.a === undefined);
                test(wd.s === undefined);

                out.writeLine("ok");
                out.write("testing optional parameters... ");

                return initial.opByte(); // same as initial.opByte(undefined);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opByte(56);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === 56);
                test(p2 === 56);
                return initial.opBool();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opBool(true);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === true);
                test(p2 === true);
                return initial.opShort();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opShort(56);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === 56);
                test(p2 === 56);
                return initial.opInt();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opInt(56);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === 56);
                test(p2 === 56);
                return initial.opLong();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opLong(new Ice.Long(0, 56));
            }
        ).then(
            function(p1, p2)
            {
                test(p1.equals(new Ice.Long(0, 56)));
                test(p2.equals(new Ice.Long(0, 56)));
                return initial.opFloat();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opFloat(1.0);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === 1.0);
                test(p2 === 1.0);
                return initial.opDouble();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opDouble(1.0);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === 1.0);
                test(p2 === 1.0);
                return initial.opString();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opString("test");
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === "test");
                test(p2 === "test");
                return initial.opMyEnum();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opMyEnum(Test.MyEnum.MyEnumMember);
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === Test.MyEnum.MyEnumMember);
                test(p2 === Test.MyEnum.MyEnumMember);
                return initial.opMyEnum(null); // Test null enum
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === Test.MyEnum.MyEnumMember);
                test(p2 === Test.MyEnum.MyEnumMember);
                return initial.opSmallStruct();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opSmallStruct(new Test.SmallStruct(56));
            }
        ).then(
            function(p1, p2)
            {
                test(p1.equals(new Test.SmallStruct(56)));
                test(p2.equals(new Test.SmallStruct(56)));
                return initial.opSmallStruct(null); // Test null struct
            }
        ).then(
            function(p1, p2)
            {
                test(p1.equals(new Test.SmallStruct(0)));
                test(p2.equals(new Test.SmallStruct(0)));
                return initial.opFixedStruct();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opFixedStruct(new Test.FixedStruct(56));
            }
        ).then(
            function(p1, p2)
            {
                test(p1.equals(new Test.FixedStruct(56)));
                test(p2.equals(new Test.FixedStruct(56)));

                return initial.opVarStruct();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opVarStruct(new Test.VarStruct("test"));
            }
        ).then(
            function(p1, p2)
            {
                test(p1.equals(new Test.VarStruct("test")));
                test(p2.equals(new Test.VarStruct("test")));
                return initial.opOneOptional();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opOneOptional(new Test.OneOptional(58));
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === p2);
                test(p2.a === 58);
                return initial.opOneOptionalProxy();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                return initial.opOneOptionalProxy(
                    Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")));
            }
        ).then(
            function(p1, p2)
            {
                var p3 = Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"));
                test(p1.equals(p3));
                test(p2.equals(p3));
                return initial.opByteSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = 56; }
                return initial.opByteSeq(Ice.Buffer.createNative(data));
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i] === 56);
                    test(p2[i] === 56);
                }
                return initial.opBoolSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = true; }
                return initial.opBoolSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i] === true);
                    test(p2[i] === true);
                }
                return initial.opShortSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = 56; }
                return initial.opShortSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i] === 56);
                    test(p2[i] === 56);
                }
                return initial.opIntSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = 56; }
                return initial.opIntSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i] === 56);
                    test(p2[i] === 56);
                }
                return initial.opLongSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = new Ice.Long(0, 56); }
                return initial.opLongSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i].equals(new Ice.Long(0, 56)));
                    test(p2[i].equals(new Ice.Long(0, 56)));
                }
                return initial.opFloatSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = 1.0; }
                return initial.opFloatSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i] === 1.0);
                    test(p2[i] === 1.0);
                }
                return initial.opDoubleSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = 1.0; }
                return initial.opDoubleSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i] === 1.0);
                    test(p2[i] === 1.0);
                }
                return initial.opStringSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = "test1"; }
                return initial.opStringSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i] == "test1");
                    test(p2[i] == "test1");
                }
                return initial.opSmallStructSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = new Test.SmallStruct(); }
                return initial.opSmallStructSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                var s = new Test.SmallStruct();
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i].equals(s));
                    test(p2[i].equals(s));
                }
                return initial.opFixedStructSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = new Test.FixedStruct(); }
                return initial.opFixedStructSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                var s = new Test.FixedStruct();
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i].equals(s));
                    test(p2[i].equals(s));
                }
                return initial.opVarStructSeq();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = [];
                for(var i = 0; i < 100; ++i){ data[i] = new Test.VarStruct(""); }
                return initial.opVarStructSeq(data);
            }
        ).then(
            function(p1, p2)
            {
                var s = new Test.VarStruct("");
                test(p1.length === 100);
                test(p2.length === 100);
                for(var i = 0; i < 100; ++i)
                {
                    test(p1[i].equals(s));
                    test(p2[i].equals(s));
                }
                return initial.opIntIntDict();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = new Ice.HashMap();
                data.set(1, 2);
                data.set(2, 3);
                return initial.opIntIntDict(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.equals(p2));
                return initial.opStringIntDict();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = new Ice.HashMap();
                data.set("1", 1);
                data.set("2", 2);
                return initial.opStringIntDict(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.equals(p2));
                return initial.opIntOneOptionalDict();
            }
        ).then(
            function(p1, p2)
            {
                test(p1 === undefined);
                test(p2 === undefined);
                var data = new Ice.HashMap();
                data.set(1, new Test.OneOptional(58));
                data.set(2, new Test.OneOptional(59));
                return initial.opIntOneOptionalDict(data);
            }
        ).then(
            function(p1, p2)
            {
                test(p1.get(1).a === 58 && p2.get(2).a === 59);

                out.writeLine("ok");
                out.write("testing exception optionals... ");

                return initial.opOptionalException();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.OptionalException);
                test(ex.a === undefined);
                test(ex.b === undefined);
                test(ex.o === undefined);

                return initial.opOptionalException(30, "test", new Test.OneOptional(53));
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.OptionalException);
                test(ex.a === 30);
                test(ex.b == "test");
                test(ex.o.a == 53);
                return initial.opDerivedException();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.DerivedException);
                test(ex.a === undefined);
                test(ex.b === undefined);
                test(ex.o === undefined);
                test(ex.ss === undefined);
                test(ex.o2 === undefined);
                return initial.opDerivedException(30, "test2", new Test.OneOptional(53));
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.DerivedException);
                test(ex.a === 30);
                test(ex.b == "test2");
                test(ex.o.a === 53);
                test(ex.ss == "test2");
                test(ex.o2.a === 53);

                out.writeLine("ok");
                return initial.shutdown();
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

    var run = function(out, id)
    {
        var c = Ice.initialize(id);
        return Promise.try(
            function()
            {
                return allTests(out, c, Test);
            }
        ).finally(
            function()
            {
                return c.destroy();
            }
        );
    };
    exports.__clientAllTests__ = allTests;
    exports.__test__ = run;
    exports.__runServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
