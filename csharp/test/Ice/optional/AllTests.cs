//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.Collections.Generic;

namespace Ice
{
    namespace optional
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.InitialPrx allTests(global::Test.TestHelper helper)
            {
                var communicator = helper.communicator();
                // FactoryI factory = new FactoryI();
                // communicator.getValueFactoryManager().add(factory.create, "");

                var output = helper.getWriter();
                var initial = Test.InitialPrx.Parse($"initial:{helper.getTestEndpoint(0)}", communicator);

                output.Write("testing optional data members... ");
                output.Flush();

                Test.OneOptional oo1 = new Test.OneOptional();
                test(!oo1.a.HasValue);
                oo1.a = 15;
                test(oo1.a.HasValue && oo1.a == 15);

                Test.OneOptional oo2 = new Test.OneOptional(16);
                test(oo2.a.HasValue && oo2.a == 16);

                Test.MultiOptional mo1 = new Test.MultiOptional();
                mo1.a = 15;
                mo1.b = true;
                mo1.c = 19;
                mo1.d = 78;
                mo1.e = 99;
                mo1.f = (float)5.5;
                mo1.g = 1.0;
                mo1.h = "test";
                mo1.i = Test.MyEnum.MyEnumMember;
                mo1.j = IObjectPrx.Parse("test", communicator);
                mo1.k = mo1;
                mo1.bs = new byte[] { 5 };
                mo1.ss = new string[] { "test", "test2" };
                mo1.iid = new Dictionary<int, int>();
                mo1.iid.Add(4, 3);
                mo1.sid = new Dictionary<string, int>();
                mo1.sid.Add("test", 10);
                Test.FixedStruct fs = new Test.FixedStruct();
                fs.m = 78;
                mo1.fs = fs;
                Test.VarStruct vs = new Test.VarStruct();
                vs.m = "hello";
                mo1.vs = vs;

                mo1.shs = new short[] { 1 };
                mo1.es = new Test.MyEnum[] { Test.MyEnum.MyEnumMember, Test.MyEnum.MyEnumMember };
                mo1.fss = new Test.FixedStruct[] { fs };
                mo1.vss = new Test.VarStruct[] { vs };
                mo1.oos = new Test.OneOptional[] { oo1 };
                mo1.oops = new IObjectPrx[] { IObjectPrx.Parse("test", communicator) };

                mo1.ied = new Dictionary<int, Test.MyEnum>();
                mo1.ied.Add(4, Test.MyEnum.MyEnumMember);
                mo1.ifsd = new Dictionary<int, Test.FixedStruct>();
                mo1.ifsd.Add(4, fs);
                mo1.ivsd = new Dictionary<int, Test.VarStruct>();
                mo1.ivsd.Add(5, vs);
                mo1.iood = new Dictionary<int, Test.OneOptional>();
                mo1.iood.Add(5, new Test.OneOptional(15));
                mo1.ioopd = new Dictionary<int, IObjectPrx>();
                mo1.ioopd.Add(5, IObjectPrx.Parse("test", communicator));

                mo1.bos = new bool[] { false, true, false };
                mo1.ser = new Test.SerializableClass(56);

                test(mo1.a == 15);
                test(mo1.b == true);
                test(mo1.c == 19);
                test(mo1.d == 78);
                test(mo1.e == 99);
                test(mo1.f == (float)5.5);
                test(mo1.g == 1.0);
                test(mo1.h.Equals("test"));
                test(mo1.i == Test.MyEnum.MyEnumMember);
                test(mo1.j.Equals(IObjectPrx.Parse("test", communicator)));
                test(mo1.k == mo1);
                test(Collections.Equals(mo1.bs, new byte[] { 5 }));
                test(Collections.Equals(mo1.ss, new string[] { "test", "test2" }));
                test(mo1.iid[4] == 3);
                test(mo1.sid["test"] == 10);
                test(mo1.fs.Equals(new Test.FixedStruct(78)));
                test(mo1.vs.Equals(new Test.VarStruct("hello")));

                test(mo1.shs[0] == 1);
                test(mo1.es[0] == Test.MyEnum.MyEnumMember && mo1.es[1] == Test.MyEnum.MyEnumMember);
                test(mo1.fss[0].Equals(new Test.FixedStruct(78)));
                test(mo1.vss[0].Equals(new Test.VarStruct("hello")));
                test(mo1.oos[0] == oo1);
                test(mo1.oops[0].Equals(IObjectPrx.Parse("test", communicator)));

                test(mo1.ied[4] == Test.MyEnum.MyEnumMember);
                test(mo1.ifsd[4].Equals(new Test.FixedStruct(78)));
                test(mo1.ivsd[5].Equals(new Test.VarStruct("hello")));
                test(mo1.iood[5].a == 15);
                test(mo1.ioopd[5].Equals(IObjectPrx.Parse("test", communicator)));

                test(Collections.Equals(mo1.bos, new bool[] { false, true, false }));
                test(mo1.ser.Equals(new Test.SerializableClass(56)));

                output.WriteLine("ok");

                output.Write("testing marshaling... ");
                output.Flush();

                Test.OneOptional oo4 = (Test.OneOptional)initial.pingPong(new Test.OneOptional());
                test(!oo4.a.HasValue);

                Test.OneOptional oo5 = (Test.OneOptional)initial.pingPong(oo1);
                test(oo1.a == oo5.a);

                Test.MultiOptional mo4 = (Test.MultiOptional)initial.pingPong(new Test.MultiOptional());
                test(mo4.a == null);
                test(mo4.b == null);
                test(mo4.c == null);
                test(mo4.d == null);
                test(mo4.e == null);
                test(mo4.f == null);
                test(mo4.g == null);
                test(mo4.h == null);
                test(mo4.i == null);
                test(mo4.j == null);
                test(mo4.k == null);
                test(mo4.bs == null);
                test(mo4.ss == null);
                test(mo4.iid == null);
                test(mo4.sid == null);
                test(mo4.fs == null);
                test(mo4.vs == null);

                test(mo4.shs == null);
                test(mo4.es == null);
                test(mo4.fss == null);
                test(mo4.vss == null);
                test(mo4.oos == null);
                test(mo4.oops == null);

                test(mo4.ied == null);
                test(mo4.ifsd == null);
                test(mo4.ivsd == null);
                test(mo4.iood == null);
                test(mo4.ioopd == null);

                test(mo4.bos == null);

                test(mo4.ser == null);

                bool supportsCsharpSerializable = initial.supportsCsharpSerializable();
                if (!supportsCsharpSerializable)
                {
                    mo1.ser = null;
                }

                Test.MultiOptional mo5 = (Test.MultiOptional)initial.pingPong(mo1);
                test(mo5.a == mo1.a);
                test(mo5.b == mo1.b);
                test(mo5.c == mo1.c);
                test(mo5.d == mo1.d);
                test(mo5.e == mo1.e);
                test(mo5.f == mo1.f);
                test(mo5.g == mo1.g);
                test(mo5.h.Equals(mo1.h));
                test(mo5.i == mo1.i);
                test(mo5.j.Equals(mo1.j));
                test(mo5.k == mo5);
                test(Collections.Equals(mo5.bs, mo1.bs));
                test(Collections.Equals(mo5.ss, mo1.ss));
                test(mo5.iid[4] == 3);
                test(mo5.sid["test"] == 10);
                test(mo5.fs.Equals(mo1.fs));
                test(mo5.vs.Equals(mo1.vs));
                test(Collections.Equals(mo5.shs, mo1.shs));
                test(mo5.es[0] == Test.MyEnum.MyEnumMember && mo1.es[1] == Test.MyEnum.MyEnumMember);
                test(mo5.fss[0].Equals(new Test.FixedStruct(78)));
                test(mo5.vss[0].Equals(new Test.VarStruct("hello")));
                test(mo5.oos[0].a == 15);
                test(mo5.oops[0].Equals(IObjectPrx.Parse("test", communicator)));

                test(mo5.ied[4] == Test.MyEnum.MyEnumMember);
                test(mo5.ifsd[4].Equals(new Test.FixedStruct(78)));
                test(mo5.ivsd[5].Equals(new Test.VarStruct("hello")));
                test(mo5.iood[5].a == 15);
                test(mo5.ioopd[5].Equals(IObjectPrx.Parse("test", communicator)));

                test(Collections.Equals(mo5.bos, new bool[] { false, true, false }));
                if (supportsCsharpSerializable)
                {
                    test(mo5.ser.Equals(new Test.SerializableClass(56)));
                }

                // Clear the first half of the optional members
                Test.MultiOptional mo6 = new Test.MultiOptional();
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

                Test.MultiOptional mo7 = (Test.MultiOptional)initial.pingPong(mo6);
                test(mo7.a == null);
                test(mo7.b.Equals(mo1.b));
                test(mo7.c == null);
                test(mo7.d.Equals(mo1.d));
                test(mo7.e == null);
                test(mo7.f.Equals(mo1.f));
                test(mo7.g == null);
                test(mo7.h.Equals(mo1.h));
                test(mo7.i == null);
                test(mo7.j.Equals(mo1.j));
                test(mo7.k == null);
                test(Collections.Equals(mo7.bs, mo1.bs));
                test(mo7.ss == null);
                test(mo7.iid[4] == 3);
                test(mo7.sid == null);
                test(mo7.fs.Equals(mo1.fs));
                test(mo7.vs == null);

                test(Collections.Equals(mo7.shs, mo1.shs));
                test(mo7.es == null);
                test(mo7.fss[0].Equals(new Test.FixedStruct(78)));
                test(mo7.vss == null);
                test(mo7.oos[0].a == 15);
                test(mo7.oops == null);

                test(mo7.ied == null);
                test(mo7.ifsd[4].Equals(new Test.FixedStruct(78)));
                test(mo7.ivsd == null);
                test(mo7.iood[5].a == 15);
                test(mo7.ioopd == null);

                test(Collections.Equals(mo7.bos, new bool[] { false, true, false }));
                test(mo7.ser == null);

                // Clear the second half of the optional members
                Test.MultiOptional mo8 = new Test.MultiOptional();
                mo8.a = mo5.a;
                mo8.c = mo5.c;
                mo8.e = mo5.e;
                mo8.g = mo5.g;
                mo8.i = mo5.i;
                mo8.k = mo8;
                mo8.ss = mo5.ss;
                mo8.sid = mo5.sid;
                mo8.vs = mo5.vs;

                mo8.es = mo5.es;
                mo8.vss = mo5.vss;
                mo8.oops = mo5.oops;

                mo8.ied = mo5.ied;
                mo8.ivsd = mo5.ivsd;
                mo8.ioopd = mo5.ioopd;
                if (supportsCsharpSerializable)
                {
                    mo8.ser = new Test.SerializableClass(56);
                }

                Test.MultiOptional mo9 = (Test.MultiOptional)initial.pingPong(mo8);
                test(mo9.a.Equals(mo1.a));
                test(!mo9.b.HasValue);
                test(mo9.c.Equals(mo1.c));
                test(!mo9.d.HasValue);
                test(mo9.e.Equals(mo1.e));
                test(!mo9.f.HasValue);
                test(mo9.g.Equals(mo1.g));
                test(mo9.h == null);
                test(mo9.i.Equals(mo1.i));
                test(mo9.j == null);
                test(mo9.k == mo9);
                test(mo9.bs == null);
                test(Collections.Equals(mo9.ss, mo1.ss));
                test(mo9.iid == null);
                test(mo9.sid["test"] == 10);
                test(mo9.fs == null);
                test(mo9.vs.Equals(mo1.vs));

                test(mo9.shs == null);
                test(mo9.es[0] == Test.MyEnum.MyEnumMember && mo9.es[1] == Test.MyEnum.MyEnumMember);
                test(mo9.fss == null);
                test(mo9.vss[0].Equals(new Test.VarStruct("hello")));
                test(mo9.oos == null);
                test(mo9.oops[0].Equals(IObjectPrx.Parse("test", communicator)));

                test(mo9.ied[4] == Test.MyEnum.MyEnumMember);
                test(mo9.ifsd == null);
                test(mo9.ivsd[5].Equals(new Test.VarStruct("hello")));
                test(mo9.iood == null);
                test(mo9.ioopd[5].Equals(IObjectPrx.Parse("test", communicator)));

                test(mo9.bos == null);
                if (supportsCsharpSerializable)
                {
                    test(mo9.ser.Equals(new Test.SerializableClass(56)));
                }

                {
                    Test.OptionalWithCustom owc1 = new Test.OptionalWithCustom();
                    owc1.l = new List<Test.SmallStruct>();
                    owc1.l.Add(new Test.SmallStruct(5));
                    owc1.l.Add(new Test.SmallStruct(6));
                    owc1.l.Add(new Test.SmallStruct(7));
                    owc1.s = new Test.ClassVarStruct(5);
                    Test.OptionalWithCustom owc2 = (Test.OptionalWithCustom)initial.pingPong(owc1);
                    test(owc2.l != null);
                    test(Collections.Equals(owc1.l, owc2.l));
                    test(owc2.s != null);
                    test(owc2.s.a == 5);
                }

                /* TODO: rewrite test without factories

                //
                // Send a request using blobjects. Upon receival, we don't read
                // any of the optional members. This ensures the optional members
                // are skipped even if the receiver knows nothing about them.
                //
                factory.setEnabled(true);
                OutputStream os = new OutputStream(communicator);
                os.StartEncapsulation();
                os.WriteValue(oo1);
                os.EndEncapsulation();
                byte[] inEncaps = os.Finished();
                byte[] outEncaps;
                test(initial.Invoke("pingPong", OperationMode.Normal, inEncaps, out outEncaps));
                InputStream @in = new InputStream(communicator, outEncaps);
                @in.StartEncapsulation();
                ReadValueCallbackI cb = new ReadValueCallbackI();
                @in.ReadValue(cb.invoke);
                @in.EndEncapsulation();
                test(cb.obj != null && cb.obj is TestValueReader);

                os = new OutputStream(communicator);
                os.StartEncapsulation();
                os.WriteValue(mo1);
                os.EndEncapsulation();
                inEncaps = os.Finished();
                test(initial.Invoke("pingPong", OperationMode.Normal, inEncaps, out outEncaps));
                @in = new InputStream(communicator, outEncaps);
                @in.StartEncapsulation();
                @in.ReadValue(cb.invoke);
                @in.EndEncapsulation();
                test(cb.obj != null && cb.obj is TestValueReader);
                factory.setEnabled(false);
                */

                byte[] outEncaps;

                //
                // Use the 1.0 encoding with operations whose only class parameters are optional.
                //
                Test.OneOptional? oo = new Test.OneOptional(53);
                initial.sendOptionalClass(true, oo);
                var initial2 = initial.Clone(encodingVersion: Util.Encoding_1_0);
                initial2.sendOptionalClass(true, oo);

                initial.returnOptionalClass(true, out oo);
                test(oo != null);
                initial2.returnOptionalClass(true, out oo);
                test(oo == null);

                Test.Recursive[] recursive1 = new Test.Recursive[1];
                recursive1[0] = new Test.Recursive();
                Test.Recursive[] recursive2 = new Test.Recursive[1];
                recursive2[0] = new Test.Recursive();
                recursive1[0].value = recursive2;
                Test.Recursive outer = new Test.Recursive();
                outer.value = recursive1;
                initial.pingPong(outer);

                Test.G g = new Test.G();
                g.gg1Opt = new Test.G1("gg1Opt");
                g.gg2 = new Test.G2(10);
                g.gg2Opt = new Test.G2(20);
                g.gg1 = new Test.G1("gg1");
                g = initial.opG(g);
                test("gg1Opt".Equals(g.gg1Opt.a));
                test(10 == g.gg2.a);
                test(20 == g.gg2Opt.a);
                test("gg1".Equals(g.gg1.a));

                initial.opVoid();

                var os = new OutputStream(communicator);
                os.StartEncapsulation();
                os.WriteOptional(1, OptionalFormat.F4);
                os.WriteInt(15);
                os.WriteOptional(1, OptionalFormat.VSize);
                os.WriteString("test");
                os.EndEncapsulation();
                var inEncaps = os.Finished();
                test(initial.Invoke("opVoid", OperationMode.Normal, inEncaps, out outEncaps));

                output.WriteLine("ok");

                output.Write("testing marshaling of large containers with fixed size elements... ");
                output.Flush();
                Test.MultiOptional mc = new Test.MultiOptional();

                mc.bs = new byte[1000];
                mc.shs = new short[300];

                mc.fss = new Test.FixedStruct[300];
                for (int i = 0; i < 300; ++i)
                {
                    mc.fss[i] = new Test.FixedStruct();
                }

                mc.ifsd = new Dictionary<int, Test.FixedStruct>();
                for (int i = 0; i < 300; ++i)
                {
                    mc.ifsd.Add(i, new Test.FixedStruct());
                }

                mc = (Test.MultiOptional)initial.pingPong(mc);
                test(mc.bs.Length == 1000);
                test(mc.shs.Length == 300);
                test(mc.fss.Length == 300);
                test(mc.ifsd.Count == 300);

                /*
                factory.setEnabled(true);
                os = new OutputStream(communicator);
                os.StartEncapsulation();
                os.WriteValue(mc);
                os.EndEncapsulation();
                inEncaps = os.Finished();
                test(initial.Invoke("pingPong", OperationMode.Normal, inEncaps, out outEncaps));
                @in = new InputStream(communicator, outEncaps);
                @in.StartEncapsulation();
                @in.ReadValue(cb.invoke);
                @in.EndEncapsulation();
                test(cb.obj != null && cb.obj is TestValueReader);
                factory.setEnabled(false);
                */

                output.WriteLine("ok");

                output.Write("testing tag marshaling... ");
                output.Flush();
                {
                    Test.B b = new Test.B();
                    Test.B b2 = (Test.B)initial.pingPong(b);
                    test(!b2.ma.HasValue);
                    test(!b2.mb.HasValue);
                    test(!b2.mc.HasValue);

                    b.ma = 10;
                    b.mb = 11;
                    b.mc = 12;
                    b.md = 13;

                    b2 = (Test.B)initial.pingPong(b);
                    test(b2.ma == 10);
                    test(b2.mb == 11);
                    test(b2.mc == 12);
                    test(b2.md == 13);

                    /*
                    factory.setEnabled(true);
                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteValue(b);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    test(initial.Invoke("pingPong", OperationMode.Normal, inEncaps, out outEncaps));
                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.ReadValue(cb.invoke);
                    @in.EndEncapsulation();
                    test(cb.obj != null);
                    factory.setEnabled(false);
                    */

                }
                output.WriteLine("ok");

                output.Write("testing marshalling of objects with optional objects...");
                output.Flush();
                {
                    Test.F f = new Test.F();

                    f.af = new Test.A();
                    f.ae = f.af;

                    Test.F rf = (Test.F)initial.pingPong(f);
                    test(rf.ae == rf.af);

                    /*
                    factory.setEnabled(true);
                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteValue(f);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    @in = new InputStream(communicator, inEncaps);
                    @in.StartEncapsulation();
                    ReadValueCallbackI rocb = new ReadValueCallbackI();
                    @in.ReadValue(rocb.invoke);
                    @in.EndEncapsulation();
                    factory.setEnabled(false);
                    rf = ((FValueReader)rocb.obj).getF();
                    test(rf.ae != null && !rf.af.HasValue);
                    */
                }
                output.WriteLine("ok");

                output.Write("testing optional with default values... ");
                output.Flush();
                {
                    Test.WD wd = (Test.WD)initial.pingPong(new Test.WD());
                    test(wd.a == 5);
                    test(wd.s.Equals("test"));
                    wd.a = null;
                    wd.s = null;
                    wd = (Test.WD)initial.pingPong(wd);
                    test(wd.a == null);
                    test(wd.s == null);
                }
                output.WriteLine("ok");

                if (communicator.Properties.getPropertyAsInt("Default.SlicedFormat") > 0)
                {
                    output.Write("testing marshaling with unknown class slices... ");
                    output.Flush();
                    {
                        Test.C c = new Test.C();
                        c.ss = "test";
                        c.ms = "testms";
                        os = new OutputStream(communicator);
                        os.StartEncapsulation();
                        os.WriteValue(c);
                        os.EndEncapsulation();
                        inEncaps = os.Finished();

                        /*
                        factory.setEnabled(true);
                        test(initial.Invoke("pingPong", OperationMode.Normal, inEncaps, out outEncaps));
                        @in = new InputStream(communicator, outEncaps);
                        @in.StartEncapsulation();
                        @in.ReadValue(cb.invoke);
                        @in.EndEncapsulation();
                        test(cb.obj is CValueReader);
                        factory.setEnabled(false);

                        factory.setEnabled(true);
                        os = new OutputStream(communicator);
                        os.StartEncapsulation();
                        Value d = new DValueWriter();
                        os.WriteValue(d);
                        os.EndEncapsulation();
                        inEncaps = os.Finished();
                        test(initial.Invoke("pingPong", OperationMode.Normal, inEncaps, out outEncaps));
                        @in = new InputStream(communicator, outEncaps);
                        @in.StartEncapsulation();
                        @in.ReadValue(cb.invoke);
                        @in.EndEncapsulation();
                        test(cb.obj != null && cb.obj is DValueReader);
                        ((DValueReader)cb.obj).check();
                        factory.setEnabled(false);
                        */
                    }
                    output.WriteLine("ok");

                    output.Write("testing optionals with unknown classes...");
                    output.Flush();
                    {
                        Test.A a = new Test.A();

                        os = new OutputStream(communicator);
                        os.StartEncapsulation();
                        os.WriteValue(a);
                        os.WriteOptional(1, OptionalFormat.Class);
                        os.WriteValue(new DValueWriter());
                        os.EndEncapsulation();
                        inEncaps = os.Finished();
                        test(initial.Invoke("opClassAndUnknownOptional", OperationMode.Normal, inEncaps,
                                                out outEncaps));

                        var @in = new InputStream(communicator, outEncaps);
                        @in.StartEncapsulation();
                        @in.EndEncapsulation();
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing optional parameters... ");
                output.Flush();
                {
                    byte? p1 = null;
                    byte? p3;
                    byte? p2 = initial.opByte(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opByte(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = 56;
                    p2 = initial.opByte(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    var r = initial.opByteAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);
                    p2 = initial.opByte(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    r = initial.opByteAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);

                    p2 = initial.opByte(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteByte(2, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opByte", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadByte(1) == 56);
                    test(@in.ReadByte(3) == 56);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    bool? p1 = null;
                    bool? p3;
                    bool? p2 = initial.opBool(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opBool(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = true;
                    p2 = initial.opBool(p1, out p3);
                    test(p2 == true && p3 == true);
                    var r = initial.opBoolAsync(p1).Result;
                    test(r.returnValue == true && r.p3 == true);
                    p2 = initial.opBool(true, out p3);
                    test(p2 == true && p3 == true);
                    r = initial.opBoolAsync(true).Result;
                    test(r.returnValue == true && r.p3 == true);

                    p2 = initial.opBool(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteBool(2, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opBool", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadBool(1) == true);
                    test(@in.ReadBool(3) == true);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    short? p1 = null;
                    short? p3;
                    short? p2 = initial.opShort(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opShort(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = 56;
                    p2 = initial.opShort(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    var r = initial.opShortAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);
                    p2 = initial.opShort(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    r = initial.opShortAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);

                    p2 = initial.opShort(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteShort(2, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opShort", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadShort(1) == 56);
                    test(@in.ReadShort(3) == 56);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    int? p1 = null;
                    int? p3;
                    int? p2 = initial.opInt(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opInt(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = 56;
                    p2 = initial.opInt(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    var r = initial.opIntAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);
                    p2 = initial.opInt(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    r = initial.opIntAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);

                    p2 = initial.opInt(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteInt(2, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opInt", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadInt(1) == 56);
                    test(@in.ReadInt(3) == 56);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    long? p1 = null;
                    long? p3;
                    long? p2 = initial.opLong(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opLong(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = 56;
                    p2 = initial.opLong(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    var r = initial.opLongAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);
                    p2 = initial.opLong(p1, out p3);
                    test(p2 == 56 && p3 == 56);
                    r = initial.opLongAsync(p1).Result;
                    test(r.returnValue == 56 && r.p3 == 56);

                    p2 = initial.opLong(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteLong(1, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opLong", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadLong(2) == 56);
                    test(@in.ReadLong(3) == 56);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    float? p1 = null;
                    float? p3;
                    float? p2 = initial.opFloat(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opFloat(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = 1.0f;
                    p2 = initial.opFloat(p1, out p3);
                    test(p2 == 1.0f && p3 == 1.0f);
                    var r = initial.opFloatAsync(p1).Result;
                    test(r.returnValue == 1.0f && r.p3 == 1.0f);
                    p2 = initial.opFloat(p1, out p3);
                    test(p2 == 1.0f && p3 == 1.0f);
                    r = initial.opFloatAsync(p1).Result;
                    test(r.returnValue == 1.0f && r.p3 == 1.0f);

                    p2 = initial.opFloat(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteFloat(2, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opFloat", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadFloat(1) == 1.0f);
                    test(@in.ReadFloat(3) == 1.0f);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    double? p1 = null;
                    double? p3;
                    double? p2 = initial.opDouble(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opDouble(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = 1.0;
                    p2 = initial.opDouble(p1, out p3);
                    test(p2 == 1.0 && p3 == 1.0);
                    var r = initial.opDoubleAsync(p1).Result;
                    test(r.returnValue == 1.0 && r.p3 == 1.0);
                    p2 = initial.opDouble(p1, out p3);
                    test(p2 == 1.0 && p3 == 1.0);
                    r = initial.opDoubleAsync(p1).Result;
                    test(r.returnValue == 1.0 && r.p3 == 1.0);

                    p2 = initial.opDouble(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteDouble(2, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opDouble", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadDouble(1) == 1.0);
                    test(@in.ReadDouble(3) == 1.0);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    string? p1 = null;
                    string? p3;
                    string? p2 = initial.opString(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opString(null, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opString(null, out p3); // Implicitly converts to string>(null)
                    test(p2 == null && p3 == null);

                    p1 = "test";
                    p2 = initial.opString(p1, out p3);
                    test(p2 == "test" && p3 == "test");
                    var r = initial.opStringAsync(p1).Result;
                    test(r.returnValue == "test" && r.p3 == "test");
                    p2 = initial.opString(p1, out p3);
                    test(p2 == "test" && p3 == "test");
                    r = initial.opStringAsync(p1).Result;
                    test(r.returnValue == "test" && r.p3 == "test");

                    p2 = initial.opString(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteString(2, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opString", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadString(1) == "test");
                    test(@in.ReadString(3) == "test");
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.MyEnum? p1 = null;
                    Test.MyEnum? p3;
                    Test.MyEnum? p2 = initial.opMyEnum(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opMyEnum(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Test.MyEnum.MyEnumMember;
                    p2 = initial.opMyEnum(p1, out p3);
                    test(p2 == Test.MyEnum.MyEnumMember && p3 == Test.MyEnum.MyEnumMember);
                    var r = initial.opMyEnumAsync(p1).Result;
                    test(r.returnValue == Test.MyEnum.MyEnumMember && r.p3 == Test.MyEnum.MyEnumMember);
                    p2 = initial.opMyEnum(p1, out p3);
                    test(p2 == Test.MyEnum.MyEnumMember && p3 == Test.MyEnum.MyEnumMember);
                    r = initial.opMyEnumAsync(p1).Result;
                    test(r.returnValue == Test.MyEnum.MyEnumMember && r.p3 == Test.MyEnum.MyEnumMember);

                    p2 = initial.opMyEnum(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteEnum(2, (int?)p1, 0);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opMyEnum", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.Size));
                    test((Test.MyEnum)@in.ReadEnum(1) == Test.MyEnum.MyEnumMember);
                    test(@in.ReadOptional(3, OptionalFormat.Size));
                    test((Test.MyEnum)@in.ReadEnum(1) == Test.MyEnum.MyEnumMember);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.SmallStruct? p1 = null;
                    Test.SmallStruct? p3;
                    Test.SmallStruct? p2 = initial.opSmallStruct(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opSmallStruct(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Test.SmallStruct(56);
                    p2 = initial.opSmallStruct(p1, out p3);
                    test(p2!.Value.m == 56 && p3!.Value.m == 56);
                    var r = initial.opSmallStructAsync(p1).Result;
                    test(p2!.Value.m == 56 && p3!.Value.m == 56);
                    p2 = initial.opSmallStruct(p1, out p3);
                    test(p2!.Value.m == 56 && p3!.Value.m == 56);
                    r = initial.opSmallStructAsync(p1).Result;
                    test(r.returnValue!.Value.m == 56 && r.p3!.Value.m == 56);

                    p2 = initial.opSmallStruct(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(1);
                    p1.Value.ice_writeMembers(os);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opSmallStruct", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    Test.SmallStruct f = new Test.SmallStruct();
                    f.ice_readMembers(@in);
                    test(f.m == 56);
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    f.ice_readMembers(@in);
                    test(f.m == 56);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.FixedStruct? p1 = null;
                    Test.FixedStruct? p3;
                    Test.FixedStruct? p2 = initial.opFixedStruct(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opFixedStruct(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Test.FixedStruct(56);
                    p2 = initial.opFixedStruct(p1, out p3);
                    test(p2!.Value.m == 56 && p3!.Value.m == 56);
                    var r = initial.opFixedStructAsync(p1).Result;
                    test(r.returnValue!.Value.m == 56 && r.p3!.Value.m == 56);
                    p2 = initial.opFixedStruct(p1, out p3);
                    test(p2!.Value.m == 56 && p3!.Value.m == 56);
                    r = initial.opFixedStructAsync(p1).Result;
                    test(r.returnValue!.Value.m == 56 && r.p3!.Value.m == 56);

                    p2 = initial.opFixedStruct(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(4);
                    p1.Value.ice_writeMembers(os);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opFixedStruct", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    Test.FixedStruct f = new Test.FixedStruct();
                    f.ice_readMembers(@in);
                    test(f.m == 56);
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    f.ice_readMembers(@in);
                    test(f.m == 56);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.VarStruct? p1 = null;
                    Test.VarStruct? p3;
                    Test.VarStruct? p2 = initial.opVarStruct(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opVarStruct(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Test.VarStruct("test");
                    p2 = initial.opVarStruct(p1, out p3);
                    test(p2!.Value.m.Equals("test") && p3!.Value.m.Equals("test"));

                    // Test null struct
                    p2 = initial.opVarStruct(null, out p3);
                    test(p2 == null && p3 == null);

                    var r = initial.opVarStructAsync(p1).Result;
                    test(r.returnValue!.Value.m.Equals("test") && r.p3!.Value.m.Equals("test"));
                    p2 = initial.opVarStruct(p1, out p3);
                    test(p2!.Value.m.Equals("test") && p3!.Value.m.Equals("test"));
                    r = initial.opVarStructAsync(p1).Result;
                    test(r.returnValue!.Value.m.Equals("test") && r.p3!.Value.m.Equals("test"));

                    p2 = initial.opVarStruct(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.FSize);
                    int pos = os.StartSize();
                    p1.Value.ice_writeMembers(os);
                    os.EndSize(pos);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opVarStruct", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.FSize));
                    @in.skip(4);
                    Test.VarStruct v = new Test.VarStruct();
                    v.ice_readMembers(@in);
                    test(v.m.Equals("test"));
                    test(@in.ReadOptional(3, OptionalFormat.FSize));
                    @in.skip(4);
                    v.ice_readMembers(@in);
                    test(v.m.Equals("test"));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.OneOptional? p1 = null;
                    Test.OneOptional? p3;
                    Test.OneOptional? p2 = initial.opOneOptional(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opOneOptional(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Test.OneOptional(58);
                    p2 = initial.opOneOptional(p1, out p3);
                    test(p2!.a == 58 && p3!.a == 58);
                    var r = initial.opOneOptionalAsync(p1).Result;
                    test(r.returnValue!.a == 58 && r.p3!.a == 58);
                    p2 = initial.opOneOptional(p1, out p3);
                    test(p2!.a == 58 && p3!.a == 58);
                    r = initial.opOneOptionalAsync(p1).Result;
                    test(r.returnValue!.a == 58 && r.p3!.a == 58);

                    p2 = initial.opOneOptional(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.Class);
                    os.WriteValue(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opOneOptional", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.Class));
                    ReadValueCallbackI p2cb = new ReadValueCallbackI();
                    @in.ReadValue(p2cb.invoke);
                    test(@in.ReadOptional(3, OptionalFormat.Class));
                    ReadValueCallbackI p3cb = new ReadValueCallbackI();
                    @in.ReadValue(p3cb.invoke);
                    @in.EndEncapsulation();
                    test(((Test.OneOptional)p2cb.obj).a == 58 && ((Test.OneOptional)p3cb.obj).a == 58);

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    IObjectPrx? p1 = null;
                    IObjectPrx? p3;
                    IObjectPrx? p2 = initial.opOneOptionalProxy(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opOneOptionalProxy(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opOneOptionalProxy(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = IObjectPrx.Parse("test", communicator);
                    p2 = initial.opOneOptionalProxy(p1, out p3);
                    test(IObjectPrx.Equals(p1, p2) && IObjectPrx.Equals(p1, p3));

                    (p2, p3) = initial.opOneOptionalProxyAsync(p1).Result;
                    test(IObjectPrx.Equals(p1, p2) && IObjectPrx.Equals(p1, p3));

                    p2 = initial.opOneOptionalProxy(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.FSize);
                    int pos = os.StartSize();
                    os.WriteProxy(p1);
                    os.EndSize(pos);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opOneOptionalProxy", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(IObjectPrx.Equals(@in.ReadProxy(1, IObjectPrx.Factory), p1));
                    test(IObjectPrx.Equals(@in.ReadProxy(3, IObjectPrx.Factory), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    byte[]? p1 = null;
                    byte[]? p3;
                    byte[]? p2 = initial.opByteSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opByteSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 100).Select(x => (byte)56).ToArray();
                    p2 = initial.opByteSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opByteSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opByteSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opByteSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opByteSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteByteSeq(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opByteSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    test(Collections.Equals(@in.ReadByteSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    test(Collections.Equals(@in.ReadByteSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    bool[]? p1 = null;
                    bool[]? p3;
                    bool[]? p2 = initial.opBoolSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opBoolSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 100).Select(_ => true).ToArray();
                    p2 = initial.opBoolSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opBoolSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opBoolSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opBoolSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opBoolSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteBoolSeq(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opBoolSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    test(Collections.Equals(@in.ReadBoolSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    test(Collections.Equals(@in.ReadBoolSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    short[]? p1 = null;
                    short[]? p3;
                    short[]? p2 = initial.opShortSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opShortSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 100).Select(_ => (short)56).ToArray();
                    p2 = initial.opShortSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opShortSeqAsync(p1).Result;

                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opShortSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opShortSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opShortSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Length * 2 + (p1.Length > 254 ? 5 : 1));
                    os.WriteShortSeq(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opShortSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadShortSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadShortSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    int[]? p1 = null;
                    int[]? p3;
                    int[]? p2 = initial.opIntSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opIntSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 100).Select(_ => 56).ToArray();
                    p2 = initial.opIntSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opIntSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opIntSeq(p1, out p3);
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    r = initial.opIntSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opIntSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Length * 4 + (p1.Length > 254 ? 5 : 1));
                    os.WriteIntSeq(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opIntSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadIntSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadIntSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    long[]? p1 = null;
                    long[]? p3;
                    long[]? p2 = initial.opLongSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opLongSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 100).Select(_ => 56L).ToArray();
                    p2 = initial.opLongSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opLongSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opLongSeq(p1, out p3);
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    r = initial.opLongSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opLongSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Length * 8 + (p1.Length > 254 ? 5 : 1));
                    os.WriteLongSeq(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opLongSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadLongSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadLongSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    float[]? p1 = null;
                    float[]? p3;
                    float[]? p2 = initial.opFloatSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opFloatSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 100).Select(_ => 1.0f).ToArray();
                    p2 = initial.opFloatSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opFloatSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opFloatSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opFloatSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opFloatSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Length * 4 + (p1.Length > 254 ? 5 : 1));
                    os.WriteFloatSeq(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opFloatSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadFloatSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadFloatSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    double[]? p1 = null;
                    double[]? p3;
                    double[]? p2 = initial.opDoubleSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opDoubleSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 100).Select(_ => 1.0).ToArray();
                    p2 = initial.opDoubleSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opDoubleSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opDoubleSeq(p1, out p3);
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    r = initial.opDoubleSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opDoubleSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Length * 8 + (p1.Length > 254 ? 5 : 1));
                    os.WriteDoubleSeq(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opDoubleSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadDoubleSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    test(Collections.Equals(@in.ReadDoubleSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    string[]? p1 = null;
                    string[]? p3;
                    string[]? p2 = initial.opStringSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opStringSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 10).Select(_ => "test1").ToArray();
                    p2 = initial.opStringSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opStringSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opStringSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opStringSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opStringSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.FSize);
                    int pos = os.StartSize();
                    os.WriteStringSeq(p1);
                    os.EndSize(pos);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opStringSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.FSize));
                    @in.skip(4);
                    test(Collections.Equals(@in.ReadStringSeq(), p1));
                    test(@in.ReadOptional(3, OptionalFormat.FSize));
                    @in.skip(4);
                    test(Collections.Equals(@in.ReadStringSeq(), p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.SmallStruct[]? p1 = null;
                    Test.SmallStruct[]? p3;
                    Test.SmallStruct[]? p2 = initial.opSmallStructSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opSmallStructSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 10).Select(_ => new Test.SmallStruct()).ToArray();
                    p2 = initial.opSmallStructSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opSmallStructSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opSmallStructSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opSmallStructSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opSmallStructSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Length + (p1.Length > 254 ? 5 : 1));
                    Test.SmallStructSeqHelper.Write(os, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opSmallStructSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    Test.SmallStruct[] arr = Test.SmallStructSeqHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    arr = Test.SmallStructSeqHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    List<Test.SmallStruct>? p1 = null;
                    List<Test.SmallStruct>? p3;
                    List<Test.SmallStruct>? p2 = initial.opSmallStructList(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opSmallStructList(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new List<Test.SmallStruct>();
                    for (int i = 0; i < 10; ++i)
                    {
                        p1.Add(new Test.SmallStruct());
                    }
                    p2 = initial.opSmallStructList(p1, out p3);
                    test(Collections.Equals(p2, p1));
                    var r = initial.opSmallStructListAsync(p1).Result;
                    test(Collections.Equals(p2, p1));
                    p2 = initial.opSmallStructList(p1, out p3);
                    test(Collections.Equals(p2, p1));
                    r = initial.opSmallStructListAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1));

                    p2 = initial.opSmallStructList(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Count + (p1.Count > 254 ? 5 : 1));
                    Test.SmallStructListHelper.Write(os, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opSmallStructList", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    List<Test.SmallStruct> arr = Test.SmallStructListHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    arr = Test.SmallStructListHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.FixedStruct[]? p1 = null;
                    Test.FixedStruct[]? p3;
                    Test.FixedStruct[]? p2 = initial.opFixedStructSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opFixedStructSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 10).Select(_ => new Test.FixedStruct()).ToArray();
                    p2 = initial.opFixedStructSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opFixedStructSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opFixedStructSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opFixedStructSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opFixedStructSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Length * 4 + (p1.Length > 254 ? 5 : 1));
                    Test.FixedStructSeqHelper.Write(os, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opFixedStructSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    Test.FixedStruct[] arr = Test.FixedStructSeqHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    arr = Test.FixedStructSeqHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    LinkedList<Test.FixedStruct>? p1 = null;
                    LinkedList<Test.FixedStruct>? p3;
                    LinkedList<Test.FixedStruct>? p2 = initial.opFixedStructList(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opFixedStructList(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new LinkedList<Test.FixedStruct>();
                    for (int i = 0; i < 10; ++i)
                    {
                        p1.AddLast(new Test.FixedStruct());
                    }
                    p2 = initial.opFixedStructList(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opFixedStructListAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opFixedStructList(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opFixedStructListAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opFixedStructList(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Count * 4 + (p1.Count > 254 ? 5 : 1));
                    Test.FixedStructListHelper.Write(os, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opFixedStructList", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    LinkedList<Test.FixedStruct> arr = Test.FixedStructListHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    arr = Test.FixedStructListHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Test.VarStruct[]? p1 = null;
                    Test.VarStruct[]? p3;
                    Test.VarStruct[]? p2 = initial.opVarStructSeq(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opVarStructSeq(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = Enumerable.Range(0, 10).Select(_ => new Test.VarStruct("")).ToArray();
                    p2 = initial.opVarStructSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opVarStructSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opVarStructSeq(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opVarStructSeqAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opVarStructSeq(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.FSize);
                    int pos = os.StartSize();
                    Test.VarStructSeqHelper.Write(os, p1);
                    os.EndSize(pos);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opVarStructSeq", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.FSize));
                    @in.skip(4);
                    Test.VarStruct[] arr = Test.VarStructSeqHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    test(@in.ReadOptional(3, OptionalFormat.FSize));
                    @in.skip(4);
                    arr = Test.VarStructSeqHelper.Read(@in);
                    test(Collections.Equals(arr, p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                if (supportsCsharpSerializable)
                {
                    Test.SerializableClass? p1 = null;
                    Test.SerializableClass? p3;
                    Test.SerializableClass? p2 = initial.opSerializable(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opSerializable(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Test.SerializableClass(58);
                    p2 = initial.opSerializable(p1, out p3);
                    test(p2.Equals(p1) && p3.Equals(p1));
                    var r = initial.opSerializableAsync(p1).Result;
                    test(r.returnValue.Equals(p1) && r.p3.Equals(p1));
                    p2 = initial.opSerializable(p1, out p3);
                    test(p2.Equals(p1) && p3.Equals(p1));
                    r = initial.opSerializableAsync(p1).Result;
                    test(r.returnValue.Equals(p1) && r.p3.Equals(p1));

                    p2 = initial.opSerializable(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSerializable(p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opSerializable", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    Test.SerializableClass sc = Test.SerializableHelper.Read(@in);
                    test(sc.Equals(p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    sc = Test.SerializableHelper.Read(@in);
                    test(sc.Equals(p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Dictionary<int, int>? p1 = null;
                    Dictionary<int, int>? p3;
                    Dictionary<int, int>? p2 = initial.opIntIntDict(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opIntIntDict(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Dictionary<int, int>();
                    p1.Add(1, 2);
                    p1.Add(2, 3);
                    p2 = initial.opIntIntDict(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opIntIntDictAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opIntIntDict(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opIntIntDictAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opIntIntDict(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.VSize);
                    os.WriteSize(p1.Count * 8 + (p1.Count > 254 ? 5 : 1));
                    Test.IntIntDictHelper.Write(os, p1);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opIntIntDict", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.VSize));
                    @in.skipSize();
                    Dictionary<int, int> m = Test.IntIntDictHelper.Read(@in);
                    test(Collections.Equals(m, p1));
                    test(@in.ReadOptional(3, OptionalFormat.VSize));
                    @in.skipSize();
                    m = Test.IntIntDictHelper.Read(@in);
                    test(Collections.Equals(m, p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }

                {
                    Dictionary<string, int>? p1 = null;
                    Dictionary<string, int>? p3;
                    Dictionary<string, int>? p2 = initial.opStringIntDict(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opStringIntDict(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Dictionary<string, int>();
                    p1.Add("1", 1);
                    p1.Add("2", 2);
                    p2 = initial.opStringIntDict(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    var r = initial.opStringIntDictAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));
                    p2 = initial.opStringIntDict(p1, out p3);
                    test(Collections.Equals(p2, p1) && Collections.Equals(p3, p1));
                    r = initial.opStringIntDictAsync(p1).Result;
                    test(Collections.Equals(r.returnValue, p1) && Collections.Equals(r.p3, p1));

                    p2 = initial.opStringIntDict(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.FSize);
                    int pos = os.StartSize();
                    Test.StringIntDictHelper.Write(os, p1);
                    os.EndSize(pos);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opStringIntDict", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.FSize));
                    @in.skip(4);
                    Dictionary<string, int> m = Test.StringIntDictHelper.Read(@in);
                    test(Collections.Equals(m, p1));
                    test(@in.ReadOptional(3, OptionalFormat.FSize));
                    @in.skip(4);
                    m = Test.StringIntDictHelper.Read(@in);
                    test(Collections.Equals(m, p1));
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();

                    Test.F f = new Test.F();
                    f.af = new Test.A();
                    f.af.requiredA = 56;
                    f.ae = f.af;

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(1, OptionalFormat.Class);
                    os.WriteValue(f);
                    os.WriteOptional(2, OptionalFormat.Class);
                    os.WriteValue(f.ae);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();

                    @in = new InputStream(communicator, inEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(2, OptionalFormat.Class));
                    ReadValueCallbackI rocb = new ReadValueCallbackI();
                    @in.ReadValue(rocb.invoke);
                    @in.EndEncapsulation();
                    Test.A a = (Test.A)rocb.obj;
                    test(a != null && a.requiredA == 56);
                }

                {
                    Dictionary<int, Test.OneOptional>? p1 = null;
                    Dictionary<int, Test.OneOptional>? p3;
                    Dictionary<int, Test.OneOptional>? p2 = initial.opIntOneOptionalDict(p1, out p3);
                    test(p2 == null && p3 == null);
                    p2 = initial.opIntOneOptionalDict(null, out p3);
                    test(p2 == null && p3 == null);

                    p1 = new Dictionary<int, Test.OneOptional>();
                    p1.Add(1, new Test.OneOptional(58));
                    p1.Add(2, new Test.OneOptional(59));
                    p2 = initial.opIntOneOptionalDict(p1, out p3);
                    test(p2[1].a == 58 && p3[1].a == 58);
                    var r = initial.opIntOneOptionalDictAsync(p1).Result;
                    test(r.returnValue[1].a == 58 && r.p3[1].a == 58);
                    p2 = initial.opIntOneOptionalDict(p1, out p3);
                    test(p2[1].a == 58 && p3[1].a == 58);
                    r = initial.opIntOneOptionalDictAsync(p1).Result;
                    test(r.returnValue[1].a == 58 && r.p3[1].a == 58);

                    p2 = initial.opIntOneOptionalDict(null, out p3);
                    test(p2 == null && p3 == null); // Ensure out parameter is cleared.

                    os = new OutputStream(communicator);
                    os.StartEncapsulation();
                    os.WriteOptional(2, OptionalFormat.FSize);
                    int pos = os.StartSize();
                    Test.IntOneOptionalDictHelper.Write(os, p1);
                    os.EndSize(pos);
                    os.EndEncapsulation();
                    inEncaps = os.Finished();
                    initial.Invoke("opIntOneOptionalDict", OperationMode.Normal, inEncaps, out outEncaps);
                    var @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    test(@in.ReadOptional(1, OptionalFormat.FSize));
                    @in.skip(4);
                    Dictionary<int, Test.OneOptional> m = Test.IntOneOptionalDictHelper.Read(@in);
                    test(m[1].a == 58);
                    test(@in.ReadOptional(3, OptionalFormat.FSize));
                    @in.skip(4);
                    m = Test.IntOneOptionalDictHelper.Read(@in);
                    test(m[1].a == 58);
                    @in.EndEncapsulation();

                    @in = new InputStream(communicator, outEncaps);
                    @in.StartEncapsulation();
                    @in.EndEncapsulation();
                }
                output.WriteLine("ok");

                output.Write("testing exception optionals... ");
                output.Flush();
                {
                    try
                    {
                        int? a = null;
                        string? b = null;
                        Test.OneOptional? o = null;
                        initial.opOptionalException(a, b, o);
                    }
                    catch (Test.OptionalException ex)
                    {
                        test(ex.a == null);
                        test(ex.b == null);
                        test(ex.o == null);
                    }

                    try
                    {
                        int? a = 30;
                        string? b = "test";
                        Test.OneOptional? o = new Test.OneOptional(53);
                        initial.opOptionalException(a, b, o);
                    }
                    catch (Test.OptionalException ex)
                    {
                        test(ex.a == 30);
                        test(ex.b == "test");
                        test(ex.o!.a == 53);
                    }

                    try
                    {
                        //
                        // Use the 1.0 encoding with an exception whose only class members are optional.
                        //
                        int? a = 30;
                        string? b = "test";
                        Test.OneOptional? o = new Test.OneOptional(53);
                        initial2.opOptionalException(a, b, o);
                    }
                    catch (Test.OptionalException ex)
                    {
                        test(ex.a == null);
                        test(ex.b == null);
                        test(ex.o == null);
                    }

                    try
                    {
                        int? a = null;
                        string? b = null;
                        Test.OneOptional? o = null;
                        initial.opDerivedException(a, b, o);
                    }
                    catch (Test.DerivedException ex)
                    {
                        test(ex.a == null);
                        test(ex.b == null);
                        test(ex.o == null);
                        test(ex.ss == null);
                        test(ex.o2 == null);
                    }

                    try
                    {
                        int? a = 30;
                        string? b = "test2";
                        Test.OneOptional? o = new Test.OneOptional(53);
                        initial.opDerivedException(a, b, o);
                    }
                    catch (Test.DerivedException ex)
                    {
                        test(ex.a == 30);
                        test(ex.b == "test2");
                        test(ex.o!.a == 53);
                        test(ex.ss == "test2");
                        test(ex.o2!.a == 53);
                    }

                    try
                    {
                        int? a = null;
                        string? b = null;
                        Test.OneOptional? o = null;
                        initial.opRequiredException(a, b, o);
                    }
                    catch (Test.RequiredException ex)
                    {
                        test(ex.a == null);
                        test(ex.b == null);
                        test(ex.o == null);
                        test(ex.ss == "test");
                        test(ex.o2 == null);
                    }

                    try
                    {
                        int? a = 30;
                        string? b = "test2";
                        Test.OneOptional? o = new Test.OneOptional(53);
                        initial.opRequiredException(a, b, o);
                    }
                    catch (Test.RequiredException ex)
                    {
                        test(ex.a == 30);
                        test(ex.b == "test2");
                        test(ex.o!.a == 53);
                        test(ex.ss == "test2");
                        test(ex.o2!.a == 53);
                    }
                }
                output.WriteLine("ok");

                output.Write("testing optionals with marshaled results... ");
                output.Flush();
                {
                    test(initial.opMStruct1() != null);
                    test(initial.opMDict1() != null);
                    test(initial.opMSeq1() != null);
                    test(initial.opMG1() != null);

                    {
                        Test.SmallStruct? p1, p2, p3;
                        p3 = initial.opMStruct2(null, out p2);
                        test(p2 == null && p3 == null);

                        p1 = new Test.SmallStruct();
                        p3 = initial.opMStruct2(p1, out p2);
                        test(p2.Equals(p1) && p3.Equals(p1));
                    }
                    {
                        string[]? p1, p2, p3;
                        p3 = initial.opMSeq2(null, out p2);
                        test(p2 == null && p3 == null);

                        p1 = new string[1] { "hello" };
                        p3 = initial.opMSeq2(p1, out p2);
                        test(Collections.Equals(p2, p1) &&
                             Collections.Equals(p3, p1));
                    }
                    {
                        Dictionary<string, int>? p1, p2, p3;
                        p3 = initial.opMDict2(null, out p2);
                        test(p2 == null && p3 == null);

                        p1 = new Dictionary<string, int>();
                        p1["test"] = 54;
                        p3 = initial.opMDict2(p1, out p2);
                        test(Collections.Equals(p2, p1) &&
                             Collections.Equals(p3, p1));
                    }
                    {
                        Test.G? p1, p2, p3;
                        p3 = initial.opMG2(null, out p2);
                        test(p2 == null && p3 == null);

                        p1 = new Test.G();
                        p3 = initial.opMG2(p1, out p2);
                        test(p2 != null && p3 != null && p3 == p2);
                    }
                }
                output.WriteLine("ok");

                return initial;
            }

            private class DValueWriter : ValueWriter
            {
                public override void write(OutputStream @out)
                {
                    @out.StartValue(null);
                    // ::Test::D
                    @out.StartSlice("::Test::D", -1, false);
                    string s = "test";
                    @out.WriteString(s);
                    @out.WriteOptional(1, OptionalFormat.FSize);
                    string[] o = { "test1", "test2", "test3", "test4" };
                    int pos = @out.StartSize();
                    @out.WriteStringSeq(o);
                    @out.EndSize(pos);
                    Test.A a = new Test.A();
                    a.mc = 18;
                    @out.WriteOptional(1000, OptionalFormat.Class);
                    @out.WriteValue(a);
                    @out.EndSlice();
                    // ::Test::B
                    @out.StartSlice(Test.B.ice_staticId(), -1, false);
                    int v = 14;
                    @out.WriteInt(v);
                    @out.EndSlice();
                    // ::Test::A
                    @out.StartSlice(Test.A.ice_staticId(), -1, true);
                    @out.WriteInt(v);
                    @out.EndSlice();
                    @out.EndValue();
                }
            }

            /*
            private class DValueReader : ValueReader
            {
                public override void read(InputStream @in)
                {
                    @in.startValue();
                    // ::Test::D
                    @in.startSlice();
                    string s = @in.ReadString();
                    test(s.Equals("test"));
                    test(@in.ReadOptional(1, OptionalFormat.FSize));
                    @in.skip(4);
                    string[] o = @in.ReadStringSeq();
                    test(o.Length == 4 &&
                         o[0].Equals("test1") && o[1].Equals("test2") && o[2].Equals("test3") && o[3].Equals("test4"));
                    test(@in.ReadOptional(1000, OptionalFormat.Class));
                    @in.ReadValue(a.invoke);
                    @in.endSlice();
                    // ::Test::B
                    @in.startSlice();
                    @in.ReadInt();
                    @in.endSlice();
                    // ::Test::A
                    @in.startSlice();
                    @in.ReadInt();
                    @in.endSlice();
                    @in.endValue(false);
                }

                internal void check()
                {
                    test(((Test.A)a.obj).mc == 18);
                }

                private ReadValueCallbackI a = new ReadValueCallbackI();
            }

            private class FValueReader : ValueReader
            {
                public override void read(InputStream @in)
                {
                    _f = new Test.F();
                    @in.startValue();
                    @in.startSlice();
                    // Don't read af on purpose
                    //in.Read(1, _f.af);
                    @in.endSlice();
                    @in.startSlice();
                    ReadValueCallbackI rocb = new ReadValueCallbackI();
                    @in.ReadValue(rocb.invoke);
                    @in.endSlice();
                    @in.endValue(false);
                    _f.ae = (Test.A)rocb.obj;
                }

                public Test.F getF()
                {
                    return _f;
                }

                private Test.F _f;
            }

            private class FactoryI
            {
                public Value create(string typeId)
                {
                    if (!_enabled)
                    {
                        return null;
                    }

                    if (typeId.Equals(Test.OneOptional.ice_staticId()))
                    {
                        return new TestValueReader();
                    }
                    else if (typeId.Equals(Test.MultiOptional.ice_staticId()))
                    {
                        return new TestValueReader();
                    }
                    else if (typeId.Equals(Test.B.ice_staticId()))
                    {
                        return new BValueReader();
                    }
                    else if (typeId.Equals(Test.C.ice_staticId()))
                    {
                        return new CValueReader();
                    }
                    else if (typeId.Equals("::Test::D"))
                    {
                        return new DValueReader();
                    }
                    else if (typeId.Equals("::Test::F"))
                    {
                        return new FValueReader();
                    }

                    return null;
                }

                internal void setEnabled(bool enabled)
                {
                    _enabled = enabled;
                }

                private bool _enabled;
            }
            */

            private class ReadValueCallbackI
            {
                public void invoke(Value obj)
                {
                    this.obj = obj;
                }

                internal Value obj;
            }
        }
    }
}
