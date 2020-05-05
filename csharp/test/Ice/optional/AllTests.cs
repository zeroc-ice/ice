//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.optional.Test;

using System.Linq;
using System.Collections.Generic;
using Test;

namespace Ice.optional
{
    public class AllTests
    {
        public static IInitialPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);

            var output = helper.GetWriter();
            var initial = IInitialPrx.Parse($"initial:{helper.GetTestEndpoint(0)}", communicator);

            output.Write("testing optional data members... ");
            output.Flush();

            var oo1 = new OneOptional();
            TestHelper.Assert(!oo1.a.HasValue);
            oo1.a = 15;
            TestHelper.Assert(oo1.a.HasValue && oo1.a == 15);

            OneOptional oo2 = new OneOptional(16);
            TestHelper.Assert(oo2.a.HasValue && oo2.a == 16);

            MultiOptional mo1 = new MultiOptional();
            mo1.a = 15;
            mo1.b = true;
            mo1.c = 19;
            mo1.d = 78;
            mo1.e = 99;
            mo1.f = (float)5.5;
            mo1.g = 1.0;
            mo1.h = "test";
            mo1.i = MyEnum.MyEnumMember;
            mo1.k = mo1;
            mo1.bs = new byte[] { 5 };
            mo1.ss = new string[] { "test", "test2" };
            mo1.iid = new Dictionary<int, int>();
            mo1.iid.Add(4, 3);
            mo1.sid = new Dictionary<string, int>();
            mo1.sid.Add("test", 10);
            FixedStruct fs = new FixedStruct();
            fs.m = 78;
            mo1.fs = fs;
            VarStruct vs = new VarStruct();
            vs.m = "hello";
            mo1.vs = vs;

            mo1.shs = new short[] { 1 };
            mo1.es = new MyEnum[] { MyEnum.MyEnumMember, MyEnum.MyEnumMember };
            mo1.fss = new FixedStruct[] { fs };
            mo1.vss = new VarStruct[] { vs };
            mo1.oos = new OneOptional[] { oo1 };

            mo1.ied = new Dictionary<int, MyEnum>();
            mo1.ied.Add(4, MyEnum.MyEnumMember);
            mo1.ifsd = new Dictionary<int, FixedStruct>();
            mo1.ifsd.Add(4, fs);
            mo1.ivsd = new Dictionary<int, VarStruct>();
            mo1.ivsd.Add(5, vs);
            mo1.iood = new Dictionary<int, OneOptional?>();
            mo1.iood.Add(5, new OneOptional(15));

            mo1.bos = new bool[] { false, true, false };
            mo1.ser = new SerializableClass(56);

            TestHelper.Assert(mo1.a == 15);
            TestHelper.Assert(mo1.b == true);
            TestHelper.Assert(mo1.c == 19);
            TestHelper.Assert(mo1.d == 78);
            TestHelper.Assert(mo1.e == 99);
            TestHelper.Assert(mo1.f == (float)5.5);
            TestHelper.Assert(mo1.g == 1.0);
            TestHelper.Assert(mo1.h.Equals("test"));
            TestHelper.Assert(mo1.i == MyEnum.MyEnumMember);
            TestHelper.Assert(mo1.k == mo1);
            TestHelper.Assert(Enumerable.SequenceEqual(mo1.bs, new byte[] { 5 }));
            TestHelper.Assert(Enumerable.SequenceEqual(mo1.ss, new string[] { "test", "test2" }));
            TestHelper.Assert(mo1.iid[4] == 3);
            TestHelper.Assert(mo1.sid["test"] == 10);
            TestHelper.Assert(mo1.fs.Equals(new FixedStruct(78)));
            TestHelper.Assert(mo1.vs.Equals(new VarStruct("hello")));

            TestHelper.Assert(mo1.shs[0] == 1);
            TestHelper.Assert(mo1.es[0] == MyEnum.MyEnumMember && mo1.es[1] == MyEnum.MyEnumMember);
            TestHelper.Assert(mo1.fss[0].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo1.vss[0].Equals(new VarStruct("hello")));
            TestHelper.Assert(mo1.oos[0] == oo1);

            TestHelper.Assert(mo1.ied[4] == MyEnum.MyEnumMember);
            TestHelper.Assert(mo1.ifsd[4].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo1.ivsd[5].Equals(new VarStruct("hello")));
            TestHelper.Assert(mo1.iood[5]!.a == 15);

            TestHelper.Assert(Enumerable.SequenceEqual(mo1.bos, new bool[] { false, true, false }));
            TestHelper.Assert(mo1.ser.Equals(new SerializableClass(56)));

            output.WriteLine("ok");

            output.Write("testing marshaling... ");
            output.Flush();

            var oo4 = (OneOptional?)initial.pingPong(new OneOptional());
            TestHelper.Assert(oo4 != null && !oo4.a.HasValue);

            var oo5 = (OneOptional?)initial.pingPong(oo1);
            TestHelper.Assert(oo5 != null && oo1.a == oo5.a);

            var mo4 = (MultiOptional?)initial.pingPong(new MultiOptional());
            TestHelper.Assert(mo4 != null);
            TestHelper.Assert(mo4.a == null);
            TestHelper.Assert(mo4.b == null);
            TestHelper.Assert(mo4.c == null);
            TestHelper.Assert(mo4.d == null);
            TestHelper.Assert(mo4.e == null);
            TestHelper.Assert(mo4.f == null);
            TestHelper.Assert(mo4.g == null);
            TestHelper.Assert(mo4.h == null);
            TestHelper.Assert(mo4.i == null);
            TestHelper.Assert(mo4.k == null);
            TestHelper.Assert(mo4.bs == null);
            TestHelper.Assert(mo4.ss == null);
            TestHelper.Assert(mo4.iid == null);
            TestHelper.Assert(mo4.sid == null);
            TestHelper.Assert(mo4.fs == null);
            TestHelper.Assert(mo4.vs == null);

            TestHelper.Assert(mo4.shs == null);
            TestHelper.Assert(mo4.es == null);
            TestHelper.Assert(mo4.fss == null);
            TestHelper.Assert(mo4.vss == null);
            TestHelper.Assert(mo4.oos == null);

            TestHelper.Assert(mo4.ied == null);
            TestHelper.Assert(mo4.ifsd == null);
            TestHelper.Assert(mo4.ivsd == null);
            TestHelper.Assert(mo4.iood == null);

            TestHelper.Assert(mo4.bos == null);

            TestHelper.Assert(mo4.ser == null);

            bool supportsCsharpSerializable = initial.supportsCsharpSerializable();
            if (!supportsCsharpSerializable)
            {
                mo1.ser = null;
            }

            var mo5 = (MultiOptional?)initial.pingPong(mo1);
            TestHelper.Assert(mo5 != null);
            TestHelper.Assert(mo5.a == mo1.a);
            TestHelper.Assert(mo5.b == mo1.b);
            TestHelper.Assert(mo5.c == mo1.c);
            TestHelper.Assert(mo5.d == mo1.d);
            TestHelper.Assert(mo5.e == mo1.e);
            TestHelper.Assert(mo5.f == mo1.f);
            TestHelper.Assert(mo5.g == mo1.g);
            TestHelper.Assert(mo5.h!.Equals(mo1.h));
            TestHelper.Assert(mo5.i == mo1.i);
            TestHelper.Assert(mo5.k == mo5);
            TestHelper.Assert(Enumerable.SequenceEqual(mo5.bs, mo1.bs));
            TestHelper.Assert(Enumerable.SequenceEqual(mo5.ss, mo1.ss));
            TestHelper.Assert(mo5.iid != null && mo5.iid[4] == 3);
            TestHelper.Assert(mo5.sid != null && mo5.sid["test"] == 10);
            TestHelper.Assert(mo5.fs.Equals(mo1.fs));
            TestHelper.Assert(mo5.vs.Equals(mo1.vs));
            TestHelper.Assert(Enumerable.SequenceEqual(mo5.shs, mo1.shs));
            TestHelper.Assert(mo5.es != null && mo5.es[0] == MyEnum.MyEnumMember && mo1.es[1] == MyEnum.MyEnumMember);
            TestHelper.Assert(mo5.fss != null && mo5.fss[0].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo5.vss != null && mo5.vss[0].Equals(new VarStruct("hello")));
            TestHelper.Assert(mo5.oos != null && mo5.oos[0]!.a == 15);

            TestHelper.Assert(mo5.ied != null && mo5.ied[4] == MyEnum.MyEnumMember);
            TestHelper.Assert(mo5.ifsd != null && mo5.ifsd[4].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo5.ivsd != null && mo5.ivsd[5].Equals(new VarStruct("hello")));
            TestHelper.Assert(mo5.iood != null && mo5.iood[5]!.a == 15);

            TestHelper.Assert(Enumerable.SequenceEqual(mo5.bos, new bool[] { false, true, false }));
            if (supportsCsharpSerializable)
            {
                TestHelper.Assert(mo5.ser!.Equals(new SerializableClass(56)));
            }

            // Clear the first half of the optional members
            MultiOptional mo6 = new MultiOptional();
            mo6.b = mo5.b;
            mo6.d = mo5.d;
            mo6.f = mo5.f;
            mo6.h = mo5.h;
            mo6.bs = mo5.bs;
            mo6.iid = mo5.iid;
            mo6.fs = mo5.fs;
            mo6.shs = mo5.shs;
            mo6.fss = mo5.fss;
            mo6.oos = mo5.oos;
            mo6.ifsd = mo5.ifsd;
            mo6.iood = mo5.iood;
            mo6.bos = mo5.bos;

            var mo7 = (MultiOptional?)initial.pingPong(mo6);
            TestHelper.Assert(mo7 != null);
            TestHelper.Assert(mo7.a == null);
            TestHelper.Assert(mo7.b.Equals(mo1.b));
            TestHelper.Assert(mo7.c == null);
            TestHelper.Assert(mo7.d.Equals(mo1.d));
            TestHelper.Assert(mo7.e == null);
            TestHelper.Assert(mo7.f.Equals(mo1.f));
            TestHelper.Assert(mo7.g == null);
            TestHelper.Assert(mo7.h != null && mo7.h.Equals(mo1.h));
            TestHelper.Assert(mo7.i == null);
            TestHelper.Assert(mo7.k == null);
            TestHelper.Assert(Enumerable.SequenceEqual(mo7.bs, mo1.bs));
            TestHelper.Assert(mo7.ss == null);
            TestHelper.Assert(mo7.iid != null && mo7.iid[4] == 3);
            TestHelper.Assert(mo7.sid == null);
            TestHelper.Assert(mo7.fs.Equals(mo1.fs));
            TestHelper.Assert(mo7.vs == null);

            TestHelper.Assert(Enumerable.SequenceEqual(mo7.shs, mo1.shs));
            TestHelper.Assert(mo7.es == null);
            TestHelper.Assert(mo7.fss != null && mo7.fss[0].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo7.vss == null);
            TestHelper.Assert(mo7.oos != null && mo7.oos[0]!.a == 15);

            TestHelper.Assert(mo7.ied == null);
            TestHelper.Assert(mo7.ifsd != null && mo7.ifsd[4].Equals(new FixedStruct(78)));
            TestHelper.Assert(mo7.ivsd == null);
            TestHelper.Assert(mo7.iood != null && mo7.iood[5]!.a == 15);

            TestHelper.Assert(Enumerable.SequenceEqual(mo7.bos, new bool[] { false, true, false }));
            TestHelper.Assert(mo7.ser == null);

            // Clear the second half of the optional members
            var mo8 = new MultiOptional();
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

            mo8.ied = mo5.ied;
            mo8.ivsd = mo5.ivsd;
            if (supportsCsharpSerializable)
            {
                mo8.ser = new SerializableClass(56);
            }

            var mo9 = (MultiOptional?)initial.pingPong(mo8);
            TestHelper.Assert(mo9 != null);
            TestHelper.Assert(mo9.a.Equals(mo1.a));
            TestHelper.Assert(!mo9.b.HasValue);
            TestHelper.Assert(mo9.c.Equals(mo1.c));
            TestHelper.Assert(!mo9.d.HasValue);
            TestHelper.Assert(mo9.e.Equals(mo1.e));
            TestHelper.Assert(!mo9.f.HasValue);
            TestHelper.Assert(mo9.g.Equals(mo1.g));
            TestHelper.Assert(mo9.h == null);
            TestHelper.Assert(mo9.i.Equals(mo1.i));
            TestHelper.Assert(mo9.k == mo9);
            TestHelper.Assert(mo9.bs == null);
            TestHelper.Assert(Enumerable.SequenceEqual(mo9.ss, mo1.ss));
            TestHelper.Assert(mo9.iid == null);
            TestHelper.Assert(mo9.sid != null && mo9.sid["test"] == 10);
            TestHelper.Assert(mo9.fs == null);
            TestHelper.Assert(mo9.vs.Equals(mo1.vs));

            TestHelper.Assert(mo9.shs == null);
            TestHelper.Assert(mo9.es != null && mo9.es[0] == MyEnum.MyEnumMember && mo9.es[1] == MyEnum.MyEnumMember);
            TestHelper.Assert(mo9.fss == null);
            TestHelper.Assert(mo9.vss != null && mo9.vss[0].Equals(new VarStruct("hello")));
            TestHelper.Assert(mo9.oos == null);

            TestHelper.Assert(mo9.ied != null && mo9.ied[4] == MyEnum.MyEnumMember);
            TestHelper.Assert(mo9.ifsd == null);
            TestHelper.Assert(mo9.ivsd != null && mo9.ivsd[5].Equals(new VarStruct("hello")));
            TestHelper.Assert(mo9.iood == null);

            TestHelper.Assert(mo9.bos == null);
            if (supportsCsharpSerializable)
            {
                TestHelper.Assert(mo9.ser!.Equals(new SerializableClass(56)));
            }

            {
                OptionalWithCustom owc1 = new OptionalWithCustom();
                owc1.l = new List<SmallStruct>();
                owc1.l.Add(new SmallStruct(5));
                owc1.l.Add(new SmallStruct(6));
                owc1.l.Add(new SmallStruct(7));
                owc1.s = new ClassVarStruct(5);
                var owc2 = (OptionalWithCustom?)initial.pingPong(owc1);
                TestHelper.Assert(owc2 != null);
                TestHelper.Assert(owc2.l != null);
                TestHelper.Assert(Enumerable.SequenceEqual(owc1.l, owc2.l));
                TestHelper.Assert(owc2.s != null && owc2.s.Value.a == 5);
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
            os.WriteClass(oo1);
            os.EndEncapsulation();
            byte[] inEncaps = os.Finished();
            byte[] outEncaps;
            test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));
            InputStream
            responseFrame.InputStream.StartEncapsulation();
            ReadClassCallbackI cb = new ReadClassCallbackI();
            responseFrame.InputStream.ReadClass(cb.invoke);
            responseFrame.InputStream.EndEncapsulation();
            test(cb.obj != null && cb.obj is TestClassReader);

            os = new OutputStream(communicator);
            os.StartEncapsulation();
            os.WriteClass(mo1);
            os.EndEncapsulation();
            inEncaps = os.Finished();
            test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));

            responseFrame.InputStream.StartEncapsulation();
            responseFrame.InputStream.ReadClass(cb.invoke);
            responseFrame.InputStream.EndEncapsulation();
            test(cb.obj != null && cb.obj is TestClassReader);
            factory.setEnabled(false);
            */

            //
            // TODO: simplify  It was using the 1.0 encoding with operations whose
            // only class parameters were optional.
            //
            OneOptional? oo = new OneOptional(53);
            initial.sendOptionalClass(true, oo);

            oo = initial.returnOptionalClass(true);
            TestHelper.Assert(oo != null);

            Recursive[] recursive1 = new Recursive[1];
            recursive1[0] = new Recursive();
            Recursive[] recursive2 = new Recursive[1];
            recursive2[0] = new Recursive();
            recursive1[0].value = recursive2;
            Recursive outer = new Recursive();
            outer.value = recursive1;
            initial.pingPong(outer);

            G? g = new G();
            g.gg1Opt = new G1("gg1Opt");
            g.gg2 = new G2(10);
            g.gg2Opt = new G2(20);
            g.gg1 = new G1("gg1");
            g = initial.opG(g);
            TestHelper.Assert(g != null);
            TestHelper.Assert("gg1Opt".Equals(g.gg1Opt!.a));
            TestHelper.Assert(10 == g.gg2!.a);
            TestHelper.Assert(20 == g.gg2Opt!.a);
            TestHelper.Assert("gg1".Equals(g.gg1!.a));

            initial.opVoid();

            var requestFrame = OutgoingRequestFrame.WithParamList(
                initial, "opVoid", idempotent: false, format: null, context: null, (15, "test"),
                (OutputStream ostr, (int n, string s) value) =>
                {
                    ostr.WriteTaggedInt(1, value.n);
                    ostr.WriteTaggedString(1, value.s); // duplicate tag ignored by the server
                });

            TestHelper.Assert(initial.Invoke(requestFrame).ReplyStatus == 0);

            output.WriteLine("ok");

            output.Write("testing marshaling of large containers with fixed size elements... ");
            output.Flush();
            MultiOptional? mc = new MultiOptional();

            mc.bs = new byte[1000];
            mc.shs = new short[300];

            mc.fss = new FixedStruct[300];
            for (int i = 0; i < 300; ++i)
            {
                mc.fss[i] = new FixedStruct();
            }

            mc.ifsd = new Dictionary<int, FixedStruct>();
            for (int i = 0; i < 300; ++i)
            {
                mc.ifsd.Add(i, new FixedStruct());
            }

            mc = (MultiOptional?)initial.pingPong(mc);
            TestHelper.Assert(mc != null);
            TestHelper.Assert(mc.bs!.Length == 1000);
            TestHelper.Assert(mc.shs!.Length == 300);
            TestHelper.Assert(mc.fss!.Length == 300);
            TestHelper.Assert(mc.ifsd!.Count == 300);

            /*
            factory.setEnabled(true);
            os = new OutputStream(communicator);
            os.StartEncapsulation();
            os.WriteClass(mc);
            os.EndEncapsulation();
            inEncaps = os.Finished();
            test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));

            responseFrame.InputStream.StartEncapsulation();
            responseFrame.InputStream.ReadClass(cb.invoke);
            responseFrame.InputStream.EndEncapsulation();
            test(cb.obj != null && cb.obj is TestClassReader);
            factory.setEnabled(false);
            */

            output.WriteLine("ok");

            output.Write("testing tag marshaling... ");
            output.Flush();
            {
                B? b = new B();
                var b2 = (B?)initial.pingPong(b);
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(!b2.ma.HasValue);
                TestHelper.Assert(!b2.mb.HasValue);
                TestHelper.Assert(!b2.mc.HasValue);

                b.ma = 10;
                b.mb = 11;
                b.mc = 12;
                b.md = 13;

                b2 = (B?)initial.pingPong(b);
                TestHelper.Assert(b2 != null);
                TestHelper.Assert(b2.ma == 10);
                TestHelper.Assert(b2.mb == 11);
                TestHelper.Assert(b2.mc == 12);
                TestHelper.Assert(b2.md == 13);

                /*
                factory.setEnabled(true);
                os = new OutputStream(communicator);
                os.StartEncapsulation();
                os.WriteClass(b);
                os.EndEncapsulation();
                inEncaps = os.Finished();
                test(initial.Invoke("pingPong", idempotent: false, inEncaps, out outEncaps));

                responseFrame.InputStream.StartEncapsulation();
                responseFrame.InputStream.ReadClass(cb.invoke);
                responseFrame.InputStream.EndEncapsulation();
                test(cb.obj != null);
                factory.setEnabled(false);
                */

            }
            output.WriteLine("ok");

            output.Write("testing marshalling of objects with optional objects...");
            output.Flush();
            {
                F? f = new F();

                f.af = new A();
                f.ae = f.af;

                var rf = (F?)initial.pingPong(f);
                TestHelper.Assert(rf != null);
                TestHelper.Assert(rf.ae == rf.af);

                /*
                factory.setEnabled(true);
                os = new OutputStream(communicator);
                os.StartEncapsulation();
                os.WriteClass(f);
                os.EndEncapsulation();
                inEncaps = os.Finished();
                responseFrame.InputStream = new InputStream(communicator, inEncaps);
                responseFrame.InputStream.StartEncapsulation();
                ReadClassCallbackI rocb = new ReadClassCallbackI();
                responseFrame.InputStream.ReadClass(rocb.invoke);
                responseFrame.InputStream.EndEncapsulation();
                factory.setEnabled(false);
                rf = ((FClassReader)rocb.obj).getF();
                test(rf.ae != null && !rf.af.HasValue);
                */
            }
            output.WriteLine("ok");

            output.Write("testing optional with default values... ");
            output.Flush();
            {
                var wd = (WD?)initial.pingPong(new WD());
                TestHelper.Assert(wd != null);
                TestHelper.Assert(wd.a == 5);
                TestHelper.Assert(wd.s! == "test");
                wd.a = null;
                wd.s = null;
                wd = (WD?)initial.pingPong(wd);
                TestHelper.Assert(wd != null);
                TestHelper.Assert(wd.a == 5);
                TestHelper.Assert(wd.s! == "test");
            }
            output.WriteLine("ok");

            output.Write("testing tagged parameters... ");
            output.Flush();
            {
                byte? p1 = null;
                (byte? p2, byte? p3) = initial.opByte(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opByte(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = 56;
                (p2, p3) = initial.opByte(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                (byte? ReturnValue, byte? p3) r = initial.opByteAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);
                (p2, p3) = initial.opByte(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                r = initial.opByteAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);

                (p2, p3) = initial.opByte(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opByte", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, byte? p1) => ostr.WriteTaggedByte(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    {
                        byte? b1 = istr.ReadTaggedByte(1);
                        byte? b2 = istr.ReadTaggedByte(3);
                        return (b1, b2);
                    });
                TestHelper.Assert(p1 == 56);
                TestHelper.Assert(p2 == 56);
            }

            {
                bool? p1 = null;
                var (p2, p3) = initial.opBool(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opBool(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = true;
                (p2, p3) = initial.opBool(p1);
                TestHelper.Assert(p2 == true && p3 == true);
                var r = initial.opBoolAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == true && r.p3 == true);
                (p2, p3) = initial.opBool(true);
                TestHelper.Assert(p2 == true && p3 == true);
                r = initial.opBoolAsync(true).Result;
                TestHelper.Assert(r.ReturnValue == true && r.p3 == true);

                (p2, p3) = initial.opBool(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opBool", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, bool? p1) => ostr.WriteTaggedBool(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                {
                    bool? b1 = istr.ReadTaggedBool(1);
                    bool? b2 = istr.ReadTaggedBool(3);
                    return (b1, b2);
                });
                TestHelper.Assert(p2 == true);
                TestHelper.Assert(p3 == true);
            }

            {
                short? p1 = null;
                var (p2, p3) = initial.opShort(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opShort(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = 56;
                (p2, p3) = initial.opShort(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                var r = initial.opShortAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);
                (p2, p3) = initial.opShort(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                r = initial.opShortAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);

                (p2, p3) = initial.opShort(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opShort", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, short? p1) => ostr.WriteTaggedShort(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    {
                        short? s1 = istr.ReadTaggedShort(1);
                        short? s2 = istr.ReadTaggedShort(3);
                        return (s1, s2);
                    });
                TestHelper.Assert(p2 == 56);
                TestHelper.Assert(p3 == 56);
            }

            {
                int? p1 = null;
                var (p2, p3) = initial.opInt(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opInt(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = 56;
                (p2, p3) = initial.opInt(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                var r = initial.opIntAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);
                (p2, p3) = initial.opInt(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                r = initial.opIntAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);

                (p2, p3) = initial.opInt(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opInt", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, int? p1) => ostr.WriteTaggedInt(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p1, p2) = responseFrame.ReadReturnValue(istr =>
                {
                    int? i1 = istr.ReadTaggedInt(1);
                    int? i2 = istr.ReadTaggedInt(3);
                    return (i1, i2);
                });
                TestHelper.Assert(p1 == 56);
                TestHelper.Assert(p2 == 56);
            }

            {
                long? p1 = null;
                var (p2, p3) = initial.opLong(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opLong(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = 56;
                (p2, p3) = initial.opLong(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                var r = initial.opLongAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);
                (p2, p3) = initial.opLong(p1);
                TestHelper.Assert(p2 == 56 && p3 == 56);
                r = initial.opLongAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 56 && r.p3 == 56);

                (p2, p3) = initial.opLong(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opLong", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, long? p1) => ostr.WriteTaggedLong(1, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                {
                    long? l1 = istr.ReadTaggedLong(2);
                    long? l2 = istr.ReadTaggedLong(3);
                    return (l1, l2);
                });
                TestHelper.Assert(p2 == 56);
                TestHelper.Assert(p3 == 56);
            }

            {
                float? p1 = null;
                var (p2, p3) = initial.opFloat(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opFloat(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = 1.0f;
                (p2, p3) = initial.opFloat(p1);
                TestHelper.Assert(p2 == 1.0f && p3 == 1.0f);
                var r = initial.opFloatAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0f && r.p3 == 1.0f);
                (p2, p3) = initial.opFloat(p1);
                TestHelper.Assert(p2 == 1.0f && p3 == 1.0f);
                r = initial.opFloatAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0f && r.p3 == 1.0f);

                (p2, p3) = initial.opFloat(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opFloat", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, float? p1) => ostr.WriteTaggedFloat(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    {
                        float? f1 = istr.ReadTaggedFloat(1);
                        float? f2 = istr.ReadTaggedFloat(3);
                        return (f1, f2);
                    });
                TestHelper.Assert(p2 == 1.0f);
                TestHelper.Assert(p3 == 1.0f);
            }

            {
                double? p1 = null;
                var (p2, p3) = initial.opDouble(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opDouble(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = 1.0;
                (p2, p3) = initial.opDouble(p1);
                TestHelper.Assert(p2 == 1.0 && p3 == 1.0);
                var r = initial.opDoubleAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0 && r.p3 == 1.0);
                (p2, p3) = initial.opDouble(p1);
                TestHelper.Assert(p2 == 1.0 && p3 == 1.0);
                r = initial.opDoubleAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == 1.0 && r.p3 == 1.0);

                (p2, p3) = initial.opDouble(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opDouble", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, double? p1) => ostr.WriteTaggedDouble(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    {
                        double? d1 = istr.ReadTaggedDouble(1);
                        double? d2 = istr.ReadTaggedDouble(3);
                        return (d1, d2);
                    });
                TestHelper.Assert(p2 == 1.0);
                TestHelper.Assert(p3 == 1.0);
            }

            {
                string? p1 = null;
                var (p2, p3) = initial.opString(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opString(null);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opString(null); // Implicitly converts to string>(null)
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = "test";
                (p2, p3) = initial.opString(p1);
                TestHelper.Assert(p2 == "test" && p3 == "test");
                var r = initial.opStringAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == "test" && r.p3 == "test");
                (p2, p3) = initial.opString(p1);
                TestHelper.Assert(p2 == "test" && p3 == "test");
                r = initial.opStringAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == "test" && r.p3 == "test");

                (p2, p3) = initial.opString(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opString", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, string? p1) => ostr.WriteTaggedString(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                {
                    string? s1 = istr.ReadTaggedString(1);
                    string? s2 = istr.ReadTaggedString(3);
                    return (s1, s2);
                });
                TestHelper.Assert(p2 == "test");
                TestHelper.Assert(p3 == "test");
            }

            {
                MyEnum? p1 = null;
                var (p2, p3) = initial.opMyEnum(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opMyEnum(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = MyEnum.MyEnumMember;
                (p2, p3) = initial.opMyEnum(p1);
                TestHelper.Assert(p2 == MyEnum.MyEnumMember && p3 == MyEnum.MyEnumMember);
                var r = initial.opMyEnumAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == MyEnum.MyEnumMember && r.p3 == MyEnum.MyEnumMember);
                (p2, p3) = initial.opMyEnum(p1);
                TestHelper.Assert(p2 == MyEnum.MyEnumMember && p3 == MyEnum.MyEnumMember);
                r = initial.opMyEnumAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue == MyEnum.MyEnumMember && r.p3 == MyEnum.MyEnumMember);

                (p2, p3) = initial.opMyEnum(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opMyEnum", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, MyEnum? p1) => ostr.WriteTaggedEnum(2, (int?) p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedEnum(1, istr => istr.ReadMyEnum()),
                        istr.ReadTaggedEnum(3, istr => istr.ReadMyEnum())));
                TestHelper.Assert(p2 == MyEnum.MyEnumMember);
                TestHelper.Assert(p3 == MyEnum.MyEnumMember);
            }

            {
                SmallStruct? p1 = null;
                var (p2, p3) = initial.opSmallStruct(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opSmallStruct(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new SmallStruct(56);
                (p2, p3) = initial.opSmallStruct(p1);
                TestHelper.Assert(p2!.Value.m == 56 && p3!.Value.m == 56);
                var r = initial.opSmallStructAsync(p1).Result;
                TestHelper.Assert(p2!.Value.m == 56 && p3!.Value.m == 56);
                (p2, p3) = initial.opSmallStruct(p1);
                TestHelper.Assert(p2!.Value.m == 56 && p3!.Value.m == 56);
                r = initial.opSmallStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.m == 56 && r.p3!.Value.m == 56);

                (p2, p3) = initial.opSmallStruct(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opSmallStruct", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, SmallStruct? p1) => ostr.WriteTaggedStruct(2, p1, 1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeStruct(1, istr => new SmallStruct(istr)),
                        istr.ReadTaggedFixedSizeStruct(3, istr => new SmallStruct(istr))));

                TestHelper.Assert(p2!.Value.m == 56);
                TestHelper.Assert(p3!.Value.m == 56);
            }

            {
                FixedStruct? p1 = null;
                var (p2, p3) = initial.opFixedStruct(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opFixedStruct(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new FixedStruct(56);
                (p2, p3) = initial.opFixedStruct(p1);
                TestHelper.Assert(p2!.Value.m == 56 && p3!.Value.m == 56);
                var r = initial.opFixedStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.m == 56 && r.p3!.Value.m == 56);
                (p2, p3) = initial.opFixedStruct(p1);
                TestHelper.Assert(p2!.Value.m == 56 && p3!.Value.m == 56);
                r = initial.opFixedStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.m == 56 && r.p3!.Value.m == 56);

                (p2, p3) = initial.opFixedStruct(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opFixedStruct", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, FixedStruct? p1) => ostr.WriteTaggedStruct(2, p1, 4));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeStruct(1, istr => new FixedStruct(istr)),
                        istr.ReadTaggedFixedSizeStruct(3, istr => new FixedStruct(istr))));
                TestHelper.Assert(p2!.Value.m == 56);
                TestHelper.Assert(p3!.Value.m == 56);
            }

            {
                VarStruct? p1 = null;
                var (p2, p3) = initial.opVarStruct(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opVarStruct(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new VarStruct("test");
                (p2, p3) = initial.opVarStruct(p1);
                TestHelper.Assert(p2!.Value.m.Equals("test") && p3!.Value.m.Equals("test"));

                // Test null struct
                (p2, p3) = initial.opVarStruct(null);
                TestHelper.Assert(p2 == null && p3 == null);

                var r = initial.opVarStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.m.Equals("test") && r.p3!.Value.m.Equals("test"));
                (p2, p3) = initial.opVarStruct(p1);
                TestHelper.Assert(p2!.Value.m.Equals("test") && p3!.Value.m.Equals("test"));
                r = initial.opVarStructAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Value.m.Equals("test") && r.p3!.Value.m.Equals("test"));

                (p2, p3) = initial.opVarStruct(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opVarStruct", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, VarStruct? p1) =>
                    {
                        TestHelper.Assert(p1 != null);
                        ostr.WriteTaggedStruct(2, p1);
                    });

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedVariableSizeStruct(1, istr => new VarStruct(istr)),
                        istr.ReadTaggedVariableSizeStruct(3, istr => new VarStruct(istr))));
                TestHelper.Assert(p2!.Value.m.Equals("test"));
                TestHelper.Assert(p3!.Value.m.Equals("test"));

                // TODO: why are we testing this here?
                /* F f = new F();
                f.af = new A();
                f.af.requiredA = 56;
                f.ae = f.af;

                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation();
                ostr.WriteOptional(1, OptionalFormat.Class);
                ostr.WriteClass(f);
                ostr.WriteOptional(2, OptionalFormat.Class);
                ostr.WriteClass(f.ae);
                ostr.EndEncapsulation();
                var inEncaps = ostr.ToArray();

                var istr = new InputStream(communicator, inEncaps);
                istr.StartEncapsulation();
                test(istr.ReadOptional(2, OptionalFormat.Class));
                var a = istr.ReadClass<A>();
                istr.EndEncapsulation();
                test(a != null && a.requiredA == 56);*/
            }

            {
                OneOptional? p1 = null;
                var (p2, p3) = initial.opOneOptional(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opOneOptional(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new OneOptional(58);
                (p2, p3) = initial.opOneOptional(p1);
                TestHelper.Assert(p2!.a == 58 && p3!.a == 58);
                var r = initial.opOneOptionalAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.a == 58 && r.p3!.a == 58);
                (p2, p3) = initial.opOneOptional(p1);
                TestHelper.Assert(p2!.a == 58 && p3!.a == 58);
                r = initial.opOneOptionalAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.a == 58 && r.p3!.a == 58);

                (p2, p3) = initial.opOneOptional(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opOneOptional", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, OneOptional? p1) => ostr.WriteTaggedClass(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedClass<OneOptional>(1), istr.ReadTaggedClass<OneOptional>(3)));
                TestHelper.Assert(p2!.a == 58 && p3!.a == 58);
            }

            {
                IObjectPrx? p1 = null;
                var (p2, p3) = initial.opOneOptionalProxy(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opOneOptionalProxy(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opOneOptionalProxy(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = IObjectPrx.Parse("test", communicator);
                (p2, p3) = initial.opOneOptionalProxy(p1);
                TestHelper.Assert(IObjectPrx.Equals(p1, p2) && IObjectPrx.Equals(p1, p3));

                (p2, p3) = initial.opOneOptionalProxyAsync(p1).Result;
                TestHelper.Assert(IObjectPrx.Equals(p1, p2) && IObjectPrx.Equals(p1, p3));

                (p2, p3) = initial.opOneOptionalProxy(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opOneOptionalProxy", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, IObjectPrx? p1) => ostr.WriteTaggedProxy(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);

                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    {
                        IObjectPrx? prx1 = istr.ReadTaggedProxy(1, IObjectPrx.Factory);
                        IObjectPrx? prx2 = istr.ReadTaggedProxy(3, IObjectPrx.Factory);
                        return (prx1, prx2);
                    });
                TestHelper.Assert(IObjectPrx.Equals(p1, p2));
                TestHelper.Assert(IObjectPrx.Equals(p1, p3));
            }

            {
                byte[]? p1 = null;
                var (p2, p3) = initial.opByteSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opByteSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 100).Select(x => (byte)56).ToArray();
                (p2, p3) = initial.opByteSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opByteSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opByteSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opByteSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opByteSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opByteSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, byte[]? p1) => ostr.WriteTaggedFixedSizeNumericArray(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeNumericArray<byte>(1), istr.ReadTaggedFixedSizeNumericArray<byte>(3)));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                bool[]? p1 = null;
                (bool[]? p2, bool[]? p3) = initial.opBoolSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opBoolSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => true).ToArray();
                (p2, p3) = initial.opBoolSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opBoolSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opBoolSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opBoolSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opBoolSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opBoolSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, bool[]? p1) => ostr.WriteTaggedFixedSizeNumericArray(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeNumericArray<bool>(1), istr.ReadTaggedFixedSizeNumericArray<bool>(3)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                short[]? p1 = null;
                var (p2, p3) = initial.opShortSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opShortSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => (short)56).ToArray();
                (p2, p3) = initial.opShortSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opShortSeqAsync(p1).Result;

                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opShortSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opShortSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opShortSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opShortSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, short[]? p1) => ostr.WriteTaggedFixedSizeNumericArray(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeNumericArray<short>(1), istr.ReadTaggedFixedSizeNumericArray<short>(3)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                int[]? p1 = null;
                var (p2, p3) = initial.opIntSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opIntSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 56).ToArray();
                (p2, p3) = initial.opIntSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opIntSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opIntSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                r = initial.opIntSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opIntSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opIntSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, int[]? p1) => ostr.WriteTaggedFixedSizeNumericArray(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeNumericArray<int>(1), istr.ReadTaggedFixedSizeNumericArray<int>(3)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                long[]? p1 = null;
                var (p2, p3) = initial.opLongSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opLongSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 56L).ToArray();
                (p2, p3) = initial.opLongSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opLongSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opLongSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                r = initial.opLongSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opLongSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opLongSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, long[]? p1) => ostr.WriteTaggedFixedSizeNumericArray(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeNumericArray<long>(1), istr.ReadTaggedFixedSizeNumericArray<long>(3)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                float[]? p1 = null;
                var (p2, p3) = initial.opFloatSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opFloatSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 1.0f).ToArray();
                (p2, p3) = initial.opFloatSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opFloatSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opFloatSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opFloatSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opFloatSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opFloatSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, float[]? p1) => ostr.WriteTaggedFixedSizeNumericArray(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeNumericArray<float>(1), istr.ReadTaggedFixedSizeNumericArray<float>(3)));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                double[]? p1 = null;
                var (p2, p3) = initial.opDoubleSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opDoubleSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 100).Select(_ => 1.0).ToArray();
                (p2, p3) = initial.opDoubleSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opDoubleSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opDoubleSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                r = initial.opDoubleSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opDoubleSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opDoubleSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, double[]? p1) => ostr.WriteTaggedFixedSizeNumericArray(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeNumericArray<double>(1), istr.ReadTaggedFixedSizeNumericArray<double>(3)));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                string[]? p1 = null;
                var (p2, p3) = initial.opStringSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opStringSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => "test1").ToArray();
                (p2, p3) = initial.opStringSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opStringSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opStringSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opStringSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opStringSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opStringSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, string[]? p1) =>
                        ostr.WriteTaggedSequence(2, p1, (ost, s) => ostr.WriteString(s)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedVariableSizeElementArray(1, 1, istr => istr.ReadString()),
                       istr.ReadTaggedVariableSizeElementArray(3, 1, istr => istr.ReadString())));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                SmallStruct[]? p1 = null;
                var (p2, p3) = initial.opSmallStructSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opSmallStructSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => new SmallStruct()).ToArray();
                (p2, p3) = initial.opSmallStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opSmallStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opSmallStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opSmallStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opSmallStructSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opSmallStructSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, SmallStruct[]? p1) => ostr.WriteTaggedSequence(2, p1, 1,
                        (ostr, st) => ostr.WriteStruct(st)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeElementArray(1, 1, istr => new SmallStruct(istr)),
                        istr.ReadTaggedFixedSizeElementArray(3, 1, istr => new SmallStruct(istr))));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                List<SmallStruct>? p1 = null;
                var (p2, p3) = initial.opSmallStructList(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opSmallStructList(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new List<SmallStruct>();
                for (int i = 0; i < 10; ++i)
                {
                    p1.Add(new SmallStruct());
                }
                (p2, p3) = initial.opSmallStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1));
                var r = initial.opSmallStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1));
                (p2, p3) = initial.opSmallStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1));
                r = initial.opSmallStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1));

                (p2, p3) = initial.opSmallStructList(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opSmallStructList", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, List<SmallStruct>? p1) => ostr.WriteTaggedSequence(2, p1, 1,
                        (ostr, st) => ostr.WriteStruct(st)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    {
                        List<SmallStruct>? list1 =
                            istr.ReadTaggedFixedSizeElementSequence(1, 1, istr => new SmallStruct(istr)) is
                                ICollection<SmallStruct> collection1 ? new List<SmallStruct>(collection1) : null;

                        List<SmallStruct>? list2 =
                            istr.ReadTaggedFixedSizeElementSequence(3, 1, istr => new SmallStruct(istr)) is
                                ICollection<SmallStruct> collection2 ? new List<SmallStruct>(collection2) : null;

                        return (list1, list2);
                    });
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));

            }

            {
                FixedStruct[]? p1 = null;
                var (p2, p3) = initial.opFixedStructSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opFixedStructSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => new FixedStruct()).ToArray();
                (p2, p3) = initial.opFixedStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opFixedStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opFixedStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opFixedStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opFixedStructSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opFixedStructSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, FixedStruct[]? p1) => ostr.WriteTaggedSequence(2, p1, 4,
                        (ostr, st) => ostr.WriteStruct(st)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeElementArray(1, 4, istr => new FixedStruct(istr)),
                        istr.ReadTaggedFixedSizeElementArray(3, 4, istr => new FixedStruct(istr))));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                LinkedList<FixedStruct>? p1 = null;
                var (p2, p3) = initial.opFixedStructList(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opFixedStructList(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new LinkedList<FixedStruct>();
                for (int i = 0; i < 10; ++i)
                {
                    p1.AddLast(new FixedStruct());
                }
                (p2, p3) = initial.opFixedStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opFixedStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opFixedStructList(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opFixedStructListAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opFixedStructList(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opFixedStructList", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, LinkedList<FixedStruct>? p1) => ostr.WriteTaggedSequence(2, p1, 4,
                        (ostr, st) => ostr.WriteStruct(st)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    {
                        LinkedList<FixedStruct>? list1 =
                            istr.ReadTaggedFixedSizeElementSequence(1, 4, istr => new FixedStruct(istr)) is
                                ICollection<FixedStruct> collection1 ? new LinkedList<FixedStruct>(collection1) : null;

                        LinkedList<FixedStruct>? list2 =
                            istr.ReadTaggedFixedSizeElementSequence(3, 4, istr => new FixedStruct(istr)) is
                                ICollection<FixedStruct> collection2 ? new LinkedList<FixedStruct>(collection2) : null;

                        return (list1, list2);
                    });
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                VarStruct[]? p1 = null;
                var (p2, p3) = initial.opVarStructSeq(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opVarStructSeq(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = Enumerable.Range(0, 10).Select(_ => new VarStruct("")).ToArray();
                (p2, p3) = initial.opVarStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opVarStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opVarStructSeq(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opVarStructSeqAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opVarStructSeq(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opVarStructSeq", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, VarStruct[]? p1) =>
                        ostr.WriteTaggedSequence(2, p1, (ostr, vs) => ostr.WriteStruct(vs)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                      (istr.ReadTaggedVariableSizeElementArray(1, 1, istr => new VarStruct(istr)),
                        istr.ReadTaggedVariableSizeElementArray(3, 1, istr => new VarStruct(istr))));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            if (supportsCsharpSerializable)
            {
                SerializableClass? p1 = null;
                var (p2, p3) = initial.opSerializable(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opSerializable(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new SerializableClass(58);
                (p2, p3) = initial.opSerializable(p1);
                TestHelper.Assert(p2!.Equals(p1) && p3!.Equals(p1));
                var r = initial.opSerializableAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Equals(p1) && r.p3!.Equals(p1));
                (p2, p3) = initial.opSerializable(p1);
                TestHelper.Assert(p2!.Equals(p1) && p3!.Equals(p1));
                r = initial.opSerializableAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue!.Equals(p1) && r.p3!.Equals(p1));

                (p2, p3) = initial.opSerializable(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opSerializable", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, SerializableClass? p1) => ostr.WriteTaggedSerializable(2, p1));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedSerializable(1) as SerializableClass,
                        istr.ReadTaggedSerializable(3) as SerializableClass));
                TestHelper.Assert(p2!.Equals(p1));
                TestHelper.Assert(p3!.Equals(p1));
            }

            {
                Dictionary<int, int>? p1 = null;
                var (p2, p3) = initial.opIntIntDict(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opIntIntDict(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new Dictionary<int, int>();
                p1.Add(1, 2);
                p1.Add(2, 3);
                (p2, p3) = initial.opIntIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opIntIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opIntIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opIntIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opIntIntDict(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opIntIntDict", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, Dictionary<int, int>? p1) => ostr.WriteTaggedDictionary(2, p1, 8,
                        (ostr, k) => ostr.WriteInt(k), (ostr, v) => ostr.WriteInt(v)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedFixedSizeEntryDictionary(1, 8, istr => istr.ReadInt(), istr => istr.ReadInt()),
                        istr.ReadTaggedFixedSizeEntryDictionary(3, 8, istr => istr.ReadInt(), istr => istr.ReadInt())));

                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                Dictionary<string, int>? p1 = null;
                var (p2, p3) = initial.opStringIntDict(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opStringIntDict(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new Dictionary<string, int>();
                p1.Add("1", 1);
                p1.Add("2", 2);
                (p2, p3) = initial.opStringIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                var r = initial.opStringIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));
                (p2, p3) = initial.opStringIntDict(p1);
                TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                r = initial.opStringIntDictAsync(p1).Result;
                TestHelper.Assert(Enumerable.SequenceEqual(r.ReturnValue, p1) && Enumerable.SequenceEqual(r.p3, p1));

                (p2, p3) = initial.opStringIntDict(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opStringIntDict", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, Dictionary<string, int>? p1) =>
                    {
                        TestHelper.Assert(p1 != null);
                        ostr.WriteTaggedDictionary(2, p1,
                            (ostr, k) => ostr.WriteString(k), (ostr, v) => ostr.WriteInt(v));
                    });

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                      (istr.ReadTaggedVariableSizeEntryDictionary(1, 5, istr => istr.ReadString(),
                                                                        istr => istr.ReadInt()),
                            istr.ReadTaggedVariableSizeEntryDictionary(3, 5, istr => istr.ReadString(),
                                                                             istr => istr.ReadInt())));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p2));
                TestHelper.Assert(Enumerable.SequenceEqual(p1, p3));
            }

            {
                Dictionary<int, OneOptional?>? p1 = null;
                var (p2, p3) = initial.opIntOneOptionalDict(p1);
                TestHelper.Assert(p2 == null && p3 == null);
                (p2, p3) = initial.opIntOneOptionalDict(null);
                TestHelper.Assert(p2 == null && p3 == null);

                p1 = new Dictionary<int, OneOptional?>();
                p1.Add(1, new OneOptional(58));
                p1.Add(2, new OneOptional(59));
                (p2, p3) = initial.opIntOneOptionalDict(p1);
                TestHelper.Assert(p2![1]!.a == 58 && p3![1]!.a == 58);
                var r = initial.opIntOneOptionalDictAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue![1]!.a == 58 && r.p3![1]!.a == 58);
                (p2, p3) = initial.opIntOneOptionalDict(p1);
                TestHelper.Assert(p2![1]!.a == 58 && p3![1]!.a == 58);
                r = initial.opIntOneOptionalDictAsync(p1).Result;
                TestHelper.Assert(r.ReturnValue![1]!.a == 58 && r.p3![1]!.a == 58);

                (p2, p3) = initial.opIntOneOptionalDict(null);
                TestHelper.Assert(p2 == null && p3 == null); // Ensure out parameter is cleared.

                requestFrame = OutgoingRequestFrame.WithParamList(initial, "opIntOneOptionalDict", idempotent: false,
                    format: null, context: null, p1,
                    (OutputStream ostr, Dictionary<int, OneOptional?>? p1) =>
                        ostr.WriteTaggedDictionary(2, p1, (ostr, k) => ostr.WriteInt(k), (ostr, v) => ostr.WriteClass(v)));

                IncomingResponseFrame responseFrame = initial.Invoke(requestFrame);
                (p2, p3) = responseFrame.ReadReturnValue(istr =>
                    (istr.ReadTaggedVariableSizeEntryDictionary(1, 5, istr => istr.ReadInt(),
                                                                     istr => istr.ReadClass<OneOptional>()),
                        istr.ReadTaggedVariableSizeEntryDictionary(3, 5, istr => istr.ReadInt(),
                                                                         istr => istr.ReadClass<OneOptional>())));
                TestHelper.Assert(p2![1]!.a == 58);
                TestHelper.Assert(p3![1]!.a == 58);
            }
            output.WriteLine("ok");

            output.Write("testing exception optionals... ");
            output.Flush();
            {
                try
                {
                    int? a = null;
                    string? b = null;
                    OneOptional? o = null;
                    initial.opOptionalException(a, b, o);
                }
                catch (OptionalException ex)
                {
                    TestHelper.Assert(ex.a == 5);
                    TestHelper.Assert(ex.b == null);
                    TestHelper.Assert(ex.o == null);
                }

                try
                {
                    int? a = 30;
                    string? b = "test";
                    OneOptional? o = new OneOptional(53);
                    initial.opOptionalException(a, b, o);
                }
                catch (OptionalException ex)
                {
                    TestHelper.Assert(ex.a == 30);
                    TestHelper.Assert(ex.b == "test");
                    TestHelper.Assert(ex.o!.a == 53);
                }

                try
                {
                    int? a = null;
                    string? b = null;
                    OneOptional? o = null;
                    initial.opDerivedException(a, b, o);
                }
                catch (DerivedException ex)
                {
                    TestHelper.Assert(ex.a == 5);
                    TestHelper.Assert(ex.b == null);
                    TestHelper.Assert(ex.o == null);
                    TestHelper.Assert(ex.ss == "test");
                    TestHelper.Assert(ex.o2 == null);
                }

                try
                {
                    int? a = 30;
                    string? b = "test2";
                    OneOptional? o = new OneOptional(53);
                    initial.opDerivedException(a, b, o);
                }
                catch (DerivedException ex)
                {
                    TestHelper.Assert(ex.a == 30);
                    TestHelper.Assert(ex.b == "test2");
                    TestHelper.Assert(ex.o!.a == 53);
                    TestHelper.Assert(ex.ss == "test2");
                    TestHelper.Assert(ex.o2!.a == 53);
                }

                try
                {
                    int? a = null;
                    string? b = null;
                    OneOptional? o = null;
                    initial.opRequiredException(a, b, o);
                }
                catch (RequiredException ex)
                {
                    TestHelper.Assert(ex.a == 5);
                    TestHelper.Assert(ex.b == null);
                    TestHelper.Assert(ex.o == null);
                    TestHelper.Assert(ex.ss == "test");
                    TestHelper.Assert(ex.o2 == null);
                }

                try
                {
                    int? a = 30;
                    string? b = "test2";
                    OneOptional? o = new OneOptional(53);
                    initial.opRequiredException(a, b, o);
                }
                catch (RequiredException ex)
                {
                    TestHelper.Assert(ex.a == 30);
                    TestHelper.Assert(ex.b == "test2");
                    TestHelper.Assert(ex.o!.a == 53);
                    TestHelper.Assert(ex.ss == "test2");
                    TestHelper.Assert(ex.o2!.a == 53);
                }
            }
            output.WriteLine("ok");

            output.Write("testing optionals with marshaled results... ");
            output.Flush();
            {
                TestHelper.Assert(initial.opMStruct1() != null);
                TestHelper.Assert(initial.opMDict1() != null);
                TestHelper.Assert(initial.opMSeq1() != null);
                TestHelper.Assert(initial.opMG1() != null);

                {
                    SmallStruct? p1, p2, p3;
                    (p3, p2) = initial.opMStruct2(null);
                    TestHelper.Assert(p2 == null && p3 == null);

                    p1 = new SmallStruct();
                    (p3, p2) = initial.opMStruct2(p1);
                    TestHelper.Assert(p2.Equals(p1) && p3.Equals(p1));
                }
                {
                    string[]? p1, p2, p3;
                    (p3, p2) = initial.opMSeq2(null);
                    TestHelper.Assert(p2 == null && p3 == null);

                    p1 = new string[1] { "hello" };
                    (p3, p2) = initial.opMSeq2(p1);
                    TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                }
                {
                    Dictionary<string, int>? p1, p2, p3;
                    (p3, p2) = initial.opMDict2(null);
                    TestHelper.Assert(p2 == null && p3 == null);

                    p1 = new Dictionary<string, int>();
                    p1["test"] = 54;
                    (p3, p2) = initial.opMDict2(p1);
                    TestHelper.Assert(Enumerable.SequenceEqual(p2, p1) && Enumerable.SequenceEqual(p3, p1));
                }
                {
                    G? p1, p2, p3;
                    (p3, p2) = initial.opMG2(null);
                    TestHelper.Assert(p2 == null && p3 == null);

                    p1 = new G();
                    (p3, p2) = initial.opMG2(p1);
                    TestHelper.Assert(p2 != null && p3 != null && p3 == p2);
                }
            }
            output.WriteLine("ok");

            return initial;
        }
    }
}
